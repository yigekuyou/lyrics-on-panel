#ifndef MPRIS_H
#define MPRIS_H

#include <QObject>
#include <QDBusInterface>
#include <QString>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
//引入这个头文件以使用 QML_ELEMENT
#include <qqmlregistration.h>
#include <QDBusPendingCallWatcher>

inline constexpr QLatin1String MPRIS2_PATH{"/org/mpris/MediaPlayer2"};
inline constexpr QLatin1String MPRIS2_PREFIX{"org.mpris.MediaPlayer2."};
inline constexpr QLatin1String PROPERTIES_INTERFACE{"org.freedesktop.DBus.Properties"};
inline constexpr QLatin1String PLAYER_INTERFACE{"org.mpris.MediaPlayer2.Player"};
inline constexpr QLatin1String MPRIS2_INTERFACE{"org.mpris.MediaPlayer2"};
class Mpris : public QObject {
		Q_OBJECT
		QML_ELEMENT
		Q_PROPERTY(QString asText READ asText NOTIFY asTextChanged)
public:
		explicit Mpris(QObject *parent = nullptr);
		QString asText() const;
		//获取歌词
		Q_INVOKABLE void findAndGetAsText(const QString &identity = "");

private slots:
		void onServiceRegistered(const QString &serviceName);
		void onServiceUnregistered(const QString &serviceName);
		void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
		void onListNamesFinished(QDBusPendingCallWatcher *watcher);
		void onGetMetadataFinished(QDBusPendingCallWatcher *watcher);
		void onGetIdentityFinished(QDBusPendingCallWatcher *watcher);
signals:
		void asTextChanged();
private:
		void searchForMprisServices();
		void connectToMprisService(const QString &serviceName);
		void disconnectFromMprisService();

		QString m_asText;
		QString m_serviceName; // 用于存储找到的服务名
		QString m_identity;
		QDBusServiceWatcher *m_serviceWatcher; //选择全面监控
};

#endif // MPRIS_H
