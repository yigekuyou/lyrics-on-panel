#include "mpris.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

Mpris::Mpris(QObject *parent)
		: QObject(parent), m_asText(""), m_serviceName("")
{
		// 构造函数不接收服务名参数，只做初始化。
		// 属性监听将在找到服务后动态设置。
}

QString Mpris::asText() const
{
		return m_asText;
}

QString Mpris::findAndGetAsText(const QString &identity)
{
		// 清除旧的服务名和文本，以备重新搜索。
		m_serviceName = "";
		m_asText = "";

		// 使用标准 D-Bus 接口获取所有服务名称。
		QDBusConnection connection = QDBusConnection::sessionBus();
		QDBusInterface dbusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", connection);
		QDBusReply<QStringList> reply = dbusInterface.call("ListNames");

		if (!reply.isValid()) {
			qDebug() << "Failed to list D-Bus names:" << reply.error().message();
				return "";
		}

		QStringList services = reply.value();
		qDebug() << "Found D-Bus services:" << services;
		for (const QString& service : services) {
				// 检查服务名称前缀。
				if (!service.startsWith(MPRIS2_PREFIX)) {
						continue;
				}
				qDebug() << "Checking MPRIS service:" << service;
				QDBusInterface interface(
					service,
				MPRIS2_PATH,
				PROPERTIES_INTERFACE, // 使用正确的 Properties 接口
				connection
				);

				// 检查接口是否有效。
				if (!interface.isValid()) {
					qDebug() << "Interface is not valid for service:" << service;
					continue;
				}

				// 同步获取 "Identity" 属性并检查是否匹配。
				QDBusReply<QVariant> identityReply = interface.call("Get", MPRIS2_INTERFACE, "Identity");
				if (!identityReply.isValid()) {
					qDebug() << "Failed to get Identity for service:" << service << "Error:" << identityReply.error().message();
						continue;
				}
				QString playerIdentity = identityReply.value().toString();
				qDebug() << "Service" << service << "has Identity:" << playerIdentity;
				if (playerIdentity != identity) {
					qDebug() << "Identity does not match. Skipping.";
						continue;
				}

				// 如果代码执行到这里，说明我们找到了匹配的服务。
				m_serviceName = service;

				// 接下来获取元数据，并将其扁平化处理。
				QDBusReply<QVariant> metadataReply = interface.call("Get", PLAYER_INTERFACE, "Metadata");
				if (metadataReply.isValid()) {
						QVariant metadataVariant = metadataReply.value();

						// 显式将 QVariant 转换为 QDBusArgument
						const QDBusArgument &arg = metadataVariant.value<QDBusArgument>();
						QVariantMap metadata;

						// 使用 QDBusArgument 的流操作符来提取 QMap
						arg >> metadata;

						qDebug() << "Metadata QMap size:" << metadata.size();
						qDebug() << "Full metadata map:" << metadata;

						if (metadata.contains("xesam:asText")) {
								m_asText = metadata.value("xesam:asText").toString();
								qDebug() << "Found lyrics:" << m_asText;
								emit asTextChanged();
						} else {
								qDebug() << "No 'xesam:asText' key found in metadata.";
						}
				} else {
						qDebug() << "Failed to get Metadata. Error:" << metadataReply.error().message();
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

void Mpris::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
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
