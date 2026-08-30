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
//引入taglib
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/unsynchronizedlyricsframe.h>
#include <taglib/tpropertymap.h>
#include <taglib/tstring.h>
#include <taglib/tag.h>
#include <QUrl>
inline constexpr QLatin1String MPRIS2_PATH{"/org/mpris/MediaPlayer2"};
inline constexpr QLatin1String MPRIS2_PREFIX{"org.mpris.MediaPlayer2."};
inline constexpr QLatin1String PROPERTIES_INTERFACE{"org.freedesktop.DBus.Properties"};
inline constexpr QLatin1String PLAYER_INTERFACE{"org.mpris.MediaPlayer2.Player"};
inline constexpr QLatin1String MPRIS2_INTERFACE{"org.mpris.MediaPlayer2"};
class Mpris : public QObject {
		Q_OBJECT
		QML_ELEMENT
		Q_PROPERTY(QString asText READ asText NOTIFY asTextChanged)
		Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
public:
		explicit Mpris(QObject *parent = nullptr);
		QString asText() const;
		QString serviceName() const;
		//获取歌词
		Q_INVOKABLE void findAndGetAsText(const QString &identity = "");
		Q_INVOKABLE void connectToServiceByName(const QString &serviceName = "");
private slots:
		void onServiceRegistered(const QString &serviceName);
		void onServiceUnregistered(const QString &serviceName);
		void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
		void onListNamesFinished(QDBusPendingCallWatcher *watcher);
		void onGetMetadataFinished(QDBusPendingCallWatcher *watcher);
		void onGetIdentityFinished(QDBusPendingCallWatcher *watcher);
		void setServiceName(const QString &serviceName); // 设置服务名并触发连接
signals:
		void asTextChanged();
		void serviceNameChanged();
private:
		void searchForMprisServices();
		void connectToMprisService(const QString &serviceName);
		void disconnectFromMprisService();
		QString getEmbeddedLyrics(const QString& localFilePath);
		QString m_asText;
		QString m_serviceName; // 用于存储找到的服务名
		QString m_identity;
		QDBusServiceWatcher *m_serviceWatcher; //选择全面监控
};

#endif // MPRIS_H
