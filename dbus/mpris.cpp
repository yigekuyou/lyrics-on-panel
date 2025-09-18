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
			} else {
					qDebug() << "Metadata does not contain xesam:asText.";
					m_asText = "";
			}
	} else {
			qDebug() << "Async metadata fetch failed. Error:" << reply.error().message();
			m_asText = "";
	}
	emit asTextChanged();
	watcher->deleteLater();
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
						// 如果属性不存在或类型不正确，则清空歌词
						m_asText = "";
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
