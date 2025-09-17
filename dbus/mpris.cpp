#include "mpris.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

mpris::mpris(QObject *parent)
		: QObject(parent), m_asText(""), m_serviceName("")
{
		// 构造函数不接收服务名参数，只做初始化。
		// 属性监听将在找到服务后动态设置。
}

QString mpris::asText() const
{
		return m_asText;
}

QString mpris::findAndGetAsText(const QString &identity)
{
		// 清除旧的服务名和文本，以备重新搜索。
		m_serviceName = "";
		m_asText = "";

		// 使用标准 D-Bus 接口获取所有服务名称。
		QDBusConnection connection = QDBusConnection::sessionBus();
		QDBusInterface dbusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", connection);
		QDBusReply<QStringList> reply = dbusInterface.call("ListNames");

		if (!reply.isValid()) {
				return "";
		}

		QStringList services = reply.value();

		for (const QString& service : services) {
				// 嵌套层级 1: 检查服务名称前缀。
				if (!service.startsWith(MPRIS2_PREFIX)) {
						continue;
				}

				QDBusInterface interface(
						service,
						MPRIS2_PATH,
						PLAYER_INTERFACE,
						connection
				);

				// 嵌套层级 2: 检查接口是否有效。
				if (!interface.isValid()) {
						continue;
				}

				// 嵌套层级 3: 同步获取 "Identity" 属性并检查是否匹配。
				QDBusReply<QVariant> identityReply = interface.call("Get", PLAYER_INTERFACE, "Identity");
				if (!identityReply.isValid()) {
						continue;
				}
				QString playerIdentity = identityReply.value().toString();
				if (playerIdentity != identity) {
						continue;
				}

				// 如果代码执行到这里，说明我们找到了匹配的服务。
				m_serviceName = service;

				// 接下来获取元数据，并将其扁平化处理。
				QDBusReply<QVariant> metadataReply = interface.call("Get", PLAYER_INTERFACE, "Metadata");
				if (metadataReply.isValid()) {
						QVariantMap metadata = metadataReply.value().value<QVariantMap>();
						if (metadata.contains("xesam:asText")) {
								m_asText = metadata.value("xesam:asText").toString();
								emit asTextChanged();
						}
				}

				// 连接到这个特定服务的属性变化信号。
				connection.connect(
						m_serviceName,
						MPRIS2_PATH,
						PROPERTIES_INTERFACE,
						"PropertiesChanged",
						this,
						SLOT(onPropertiesChanged(QString,QVariantMap,QStringList))
				);

				return m_asText;
		}

		// 如果循环结束仍未找到匹配的服务，则返回空字符串。
		emit asTextChanged();
		return "";
}

void mpris::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
		// 嵌套层级 1: 检查接口名称是否匹配且属性列表不为空。
		if (interfaceName != PLAYER_INTERFACE || changedProperties.isEmpty()) {
				return;
		}

		// 嵌套层级 2: 处理元数据（歌曲信息）的变化。
		if (changedProperties.contains("Metadata")) {
				QVariantMap metadata = changedProperties.value("Metadata").value<QVariantMap>();

				// 嵌套层级 3: 检查元数据是否包含 "xesam:asText"。
				if (metadata.contains("xesam:asText")) {
						QString newText = metadata.value("xesam:asText").toString();
						if (m_asText != newText) {
								m_asText = newText;
								emit asTextChanged();
						}
				}
		}
}
