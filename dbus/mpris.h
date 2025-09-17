#ifndef MPRIS_H
#define MPRIS_H

#include <QObject>
#include <QDBusInterface>
#include <QString>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
// 新增：引入这个头文件以使用 QML_ELEMENT
#include <qqmlregistration.h>


inline constexpr QLatin1String MPRIS2_PATH{"/org/mpris/MediaPlayer2"};
inline constexpr QLatin1String MPRIS2_PREFIX{"org.mpris.MediaPlayer2."};
inline constexpr QLatin1String PROPERTIES_INTERFACE{"org.freedesktop.DBus.Properties"};
inline constexpr QLatin1String PLAYER_INTERFACE{"org.mpris.MediaPlayer2.Player"};

class Mpris : public QObject {
		Q_OBJECT
		QML_ELEMENT
		Q_PROPERTY(QString asText READ asText NOTIFY asTextChanged)
public:
		explicit Mpris(QObject *parent = nullptr);
		QString asText() const;
		//获取歌词
		Q_INVOKABLE QString findAndGetAsText(const QString &identity);

private slots:
		void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

signals:
		void asTextChanged();

private:
		QString m_asText;
		QString m_serviceName; // 新增成员，用于存储找到的服务名
};

#endif // MPRIS_H
