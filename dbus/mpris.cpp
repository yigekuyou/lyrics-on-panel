#include "mpris.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

Mpris::Mpris(QObject *parent)
		: QObject(parent), m_asText(""), m_serviceName("")
{
		qDebug() << "Mpris object created.";
		// 在构造函数中立即开始监听所有 MPRIS 服务
		QDBusServiceWatcher* watcher = new QDBusServiceWatcher(MPRIS2_PREFIX,
																													 QDBusConnection::sessionBus(),
																													 QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
																													 this);

		// 连接到服务注册和注销信号
		connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &Mpris::onServiceRegistered);
		connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, &Mpris::onServiceUnregistered);
		qDebug() << "Mpris object connet.";

		// 立即检查所有已注册的服务
		// 在这里不再调用,他会链接第一个
		// searchForMprisServices();
		//qDebug() << "Mpris object search.";
}

QString Mpris::asText() const
{
		qDebug() << "return asText ok ";
		return m_asText;
}
void Mpris::findAndGetAsText(const QString &identity)
{
		qDebug() << "Searching for media player with identity:" << identity;

		// 如果已经连接到目标服务，直接返回
		if (!m_serviceName.isEmpty() && m_serviceName.endsWith(identity)) {
				qDebug() << "Already connected to the desired service:" << m_serviceName;
				return;
		}

		// 断开旧的连接
		disconnectFromMprisService();
		qDebug() << "disconnect service:" ;

		// 设置新的 identity 并开始搜索
		m_identity = identity;
		qDebug() << "identity service:" << m_identity;
		searchForMprisServices();
}

void Mpris::searchForMprisServices()
{
		QDBusConnection connection = QDBusConnection::sessionBus();
		QDBusInterface dbusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", connection);

		QDBusPendingCall asyncCall = dbusInterface.asyncCall("ListNames");
		QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCall, this);

		connect(watcher, &QDBusPendingCallWatcher::finished, this, &Mpris::onListNamesFinished);
}
void Mpris::onListNamesFinished(QDBusPendingCallWatcher *watcher)
{
		QDBusPendingReply<QStringList> reply = *watcher;
		if (reply.isError()) {
				qWarning() << "Failed to list D-Bus services:" << reply.error().message();
				return;
		}

		const QStringList services = reply.value();
		for (const QString &serviceName : services) {
				if (serviceName.startsWith(MPRIS2_PREFIX)) {
						// 获取服务的 Identity 属性
						QDBusInterface interface(serviceName, MPRIS2_PATH, PROPERTIES_INTERFACE, QDBusConnection::sessionBus());
						QDBusPendingReply<QVariant> identityReply = interface.call("Get", MPRIS2_INTERFACE, "Identity");

						// 使用 QDBusPendingCallWatcher 异步处理回复
						QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(identityReply, this);
						connect(watcher, &QDBusPendingCallWatcher::finished, this, &Mpris::onGetIdentityFinished);

						// 将服务名也附加到 watcher，以便在回调中识别。
						watcher->setProperty("serviceName", serviceName);
				}
		}
		watcher->deleteLater();
}

void Mpris::onServiceRegistered(const QString &serviceName)
{
		qDebug() << "New MPRIS service registered:" << serviceName;

		// 只有在当前没有活动连接，且新服务匹配目标 identity 或 identity 为空时，才尝试连接
		if (m_serviceName.isEmpty() && serviceName.startsWith(MPRIS2_PREFIX) && (m_identity.isEmpty() || serviceName.endsWith(m_identity))) {
				qDebug() << "Mpris object created. Starting D-Bus service watcher.";
				connectToMprisService(serviceName);
		}
}
void Mpris::onServiceUnregistered(const QString &serviceName)
{
		qDebug() << "MPRIS service unregistered:" << serviceName;
		if (m_serviceName == serviceName) {
				disconnectFromMprisService();
		}
}
void Mpris::connectToMprisService(const QString &serviceName)
{
		if (m_serviceName == serviceName) {
				return;
		}
		disconnectFromMprisService();

		m_serviceName = serviceName;

		QDBusInterface mprisInterface(m_serviceName, MPRIS2_PATH, PROPERTIES_INTERFACE, QDBusConnection::sessionBus());
		QDBusPendingCall asyncCall = mprisInterface.asyncCall("Get", PLAYER_INTERFACE, "Metadata");
		QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCall, this);
		connect(watcher, &QDBusPendingCallWatcher::finished, this, &Mpris::onGetMetadataFinished);

		bool connected = QDBusConnection::sessionBus().connect(
				m_serviceName,
				MPRIS2_PATH,
				PROPERTIES_INTERFACE,
				"PropertiesChanged",
				this,
				SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
		);
		if (!connected) {
				qWarning() << "Failed to connect to PropertiesChanged signal for service:" << m_serviceName;
		}
}

void Mpris::onGetMetadataFinished(QDBusPendingCallWatcher *watcher)
{

	QDBusPendingReply<QVariant> reply = *watcher;

	if (reply.isValid()) {
			QVariant metadataVariant = reply.value();
			QVariantMap metadata;
			// 显式检查类型是否为 QVariantMap
			if (metadataVariant.isValid() && metadataVariant.typeId() == QMetaType::QVariantMap) {
					metadata = metadataVariant.value<QVariantMap>();
			}
			if (metadataVariant.canConvert<QDBusArgument>()){
			const QDBusArgument &arg = metadataVariant.value<QDBusArgument>();
			arg >> metadata; // Use the stream operator to extract the data.
			}
			qDebug() << "Full metadata map:" << metadata; // 确保这行能看到完整的QMap

			if (metadata.contains("xesam:asText")) {
					m_asText = metadata.value("xesam:asText").toString();
					qDebug() << "Async metadata fetch. Lyrics:" << m_asText;
			}  else {
					if (metadata.contains("xesam:url")) {
							QUrl mediaUrl = metadata.value("xesam:url").toUrl();
							if (mediaUrl.isLocalFile()) {
									QString localPath = mediaUrl.toLocalFile();
									qDebug() << "Found media local file path:" << localPath;

									// 重点：调用 TagLib 函数提取歌词
									m_asText = getEmbeddedLyrics(localPath);

							} else {
									qDebug() << "Media is not a local file (URL:" << mediaUrl.toString() << "). Skipping TagLib extraction.";
									// 如果不是本地文件，我们不能使用 TagLib
									m_asText = "";
							}
					} else {
							qDebug() << "Metadata does not contain xesam:url. Cannot use TagLib.";
							// 如果连 xesam:url 都没有，我们无法定位文件
							m_asText = "";
					}
			}
	} else {
			qDebug() << "Async metadata fetch failed. Error:" << reply.error().message();
			m_asText = "";
	}
	emit asTextChanged();
	watcher->deleteLater();
}
// -----------------------------------------------------------
// TagLib 歌词提取实现
// -----------------------------------------------------------
QString Mpris::getEmbeddedLyrics(const QString& localFilePath)
{
		// 1. 文件打开和 FileRef 检查
		TagLib::FileName tagLibPath = localFilePath.toLocal8Bit().constData();
		TagLib::FileRef f(tagLibPath);

		if (f.isNull() || !f.tag()) {
				qDebug() << "TagLib Error: 无法打开文件或读取标签:" << localFilePath;
				return QString();
		}

		// --- 只处理 ID3v2 USLT 帧 ---
		TagLib::ID3v2::Tag* id3v2tag = dynamic_cast<TagLib::ID3v2::Tag*>(f.tag());

		// 检查文件是否有 ID3v2 标签
		if (id3v2tag) {
				// 调试信息：输出 ID3v2 版本，帮助诊断
				qDebug() << "TagLib Debug: ID3v2 Tag found. Major version:" << id3v2tag->header()->majorVersion();

				// 尝试获取 USLT 帧列表 (v2.3/v2.4 帧 ID)
				TagLib::ID3v2::FrameList frameList = id3v2tag->frameList("USLT");

				if (frameList.isEmpty()) {
						qDebug() << "TagLib Debug: USLT frame list is empty. Checking for v2.2 'ULT' frame as well.";
						// 回退到 ID3v2.2 的帧 ID (ULT)
						frameList = id3v2tag->frameList("ULT");
				}
				// 获取 USLT 帧列表
				//TagLib::ID3v2::FrameList frameList = id3v2tag->frameList("USLT");

				if (!frameList.isEmpty()) {
						// 使用正确的命名空间 TagLib::ID3v2::
						TagLib::ID3v2::UnsynchronizedLyricsFrame* usltFrame =
								dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(frameList.front());

						if (usltFrame) {
								QString lyrics = QString::fromUtf8(usltFrame->text().toCString(true));

								// 检查歌词内容是否为空
								if (!lyrics.trimmed().isEmpty()) {
										qDebug() << "TagLib: 成功从 ID3v2 USLT 帧读取歌词。";
										qDebug() <<lyrics ;
										return lyrics; // 成功，立即返回
								}
						}
				}
		}
		// --- ID3v2 检查结束 ---
		if (f.tag()) {
						TagLib::PropertyMap properties = f.file()->properties();
						// 检查 "LYRICS" 键。TagLib::PropertyMap 返回一个 TagLib::StringList。
						if (properties.contains("LYRICS") && !properties["LYRICS"].isEmpty()) {

								// 歌词可能存储为列表中的第一个元素
								TagLib::String tagLyrics = properties["LYRICS"].front();

								if (!tagLyrics.isEmpty()) {
										QString lyrics = QString::fromUtf8(tagLyrics.toCString(true));

										if (!lyrics.trimmed().isEmpty()) {
												qDebug() << "TagLib: 成功从通用 'LYRICS' 标签读取歌词 (e.g., Vorbis Comment/FLAC)。";
												qDebug() <<lyrics ;
												return lyrics; // 成功，立即返回
										}
								}
						}
		}

		// 如果 ID3v2 标签不存在、不包含 USLT 帧，或 USLT 帧为空，则返回空字符串。
		qDebug() << "TagLib Info: 文件元数据中未找到嵌入歌词 。";
		return QString();
}
void Mpris::disconnectFromMprisService()
{
		if (!m_serviceName.isEmpty()) {
				QDBusConnection::sessionBus().disconnect(
						m_serviceName,
						MPRIS2_PATH,
						PROPERTIES_INTERFACE,
						"PropertiesChanged",
						this,
						SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
				);
				m_serviceName = "";
		}
		m_asText = "";
		emit asTextChanged();
}

void Mpris::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
		if (interfaceName == PLAYER_INTERFACE && changedProperties.contains("Metadata")) {
				qDebug() << "  Metadata property changed.";
				QVariant metadataVariant = changedProperties.value("Metadata");
				QVariantMap metadata;
				if (metadataVariant.isValid() && metadataVariant.typeId() == QMetaType::QVariantMap) {
						metadata = metadataVariant.value<QVariantMap>();
				}
				if (metadataVariant.canConvert<QDBusArgument>()){
				const QDBusArgument &arg = metadataVariant.value<QDBusArgument>();
				arg >> metadata; // Use the stream operator to extract the data.
				}
				QString oldText = m_asText;
				// 获取 xesam:asText 的 QVariant 值
				QVariant asTextVariant = metadata.value("xesam:asText");

				// 检查 QVariant 是否有效且类型是字符串
				if (asTextVariant.isValid() && asTextVariant.typeId() == QMetaType::QString) {
						m_asText = asTextVariant.toString();
						qDebug() << "  xesam:asText found in metadata. New value:" << m_asText;
				} else {
						if (metadata.contains("xesam:url")) {
								QUrl mediaUrl = metadata.value("xesam:url").toUrl();
								if (mediaUrl.isLocalFile()) {
										QString localPath = mediaUrl.toLocalFile();
										qDebug() << "Found media local file path:" << localPath;

										// 重点：调用 TagLib 函数提取歌词
										m_asText = getEmbeddedLyrics(localPath);

								} else {
										qDebug() << "Media is not a local file (URL:" << mediaUrl.toString() << "). Skipping TagLib extraction.";
										// 如果不是本地文件，我们不能使用 TagLib
										m_asText = "";
								}
						} else {
								qDebug() << "Metadata does not contain xesam:url. Cannot use TagLib.";
								// 如果连 xesam:url 都没有，我们无法定位文件
								m_asText = "";
						}
				}

				// 只有当歌词内容真正改变时才发出信号
				if (m_asText != oldText) {
						qDebug() << "  asText value has changed. Emitting asTextChanged signal.";
						emit asTextChanged();
				} else {
						qDebug() << "  asText value has not changed. Not emitting signal.";
				}
		} else {
				qDebug() << "  Metadata property not found in changed properties. Skipping.";
		}
}

void Mpris::onGetIdentityFinished(QDBusPendingCallWatcher *watcher)
{
		QDBusPendingReply<QVariant> reply = *watcher;

		if (reply.isError()) {
				qWarning() << "Failed to get service identity:" << reply.error().message();
		} else {
				QString identity = reply.value().toString();
				qDebug() << "Service identity found:" << identity;

				// 检查这个服务的 identity 是否与目标 identity 匹配
				// 如果匹配，则连接到该服务
				if (m_identity.isEmpty() || identity == m_identity) {
						QString serviceName = watcher->property("serviceName").toString();
						qDebug() << "Found matching MPRIS service:" << serviceName;
						connectToMprisService(serviceName);
				}
		}
		watcher->deleteLater();
}
