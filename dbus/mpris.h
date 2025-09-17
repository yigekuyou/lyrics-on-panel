#ifndef MPRIS_H
#define MPRIS_H

#include <QObject>
#include <QDBusInterface>
#include <QString>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>

inline constexpr QLatin1String MPRIS2_PATH{"/org/mpris/MediaPlayer2"};
inline constexpr QLatin1String MPRIS2_PREFIX{"org.mpris.MediaPlayer2."};
inline constexpr QLatin1String PROPERTIES_INTERFACE{"org.freedesktop.DBus.Properties"};
inline constexpr QLatin1String PLAYER_INTERFACE{"org.mpris.MediaPlayer2.Player"};

class mpris : public QObject {
		Q_OBJECT
		Q_PROPERTY(QString asText READ asText NOTIFY asTextChanged)

public:
		explicit mpris(QObject *parent = nullptr);
		QString asText() const;

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
