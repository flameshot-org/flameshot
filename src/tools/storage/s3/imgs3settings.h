#ifndef IMG_S3_SETTINGS_H
#define IMG_S3_SETTINGS_H

#define S3_API_IMG_PATH "v2/image/"
#define S3_GET_REMOTE_SETTINGS_TIMEOUT 10

#define S3_CONFIG_LOCAL "config.ini"
#define S3_CONFIG_PROXY "config_proxy.ini"

#include <QObject>
#include <QString>
#include <QUrl>

class QSettings;
class QNetworkProxy;
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

class ImgS3Settings : public QObject
{
    Q_OBJECT

private slots:
    void handleReplyUpdateConfigFromRemote(QNetworkReply* reply);

public:
    ImgS3Settings(QObject* parent = nullptr);

    bool getConfigRemote(int timeout = S3_GET_REMOTE_SETTINGS_TIMEOUT);
    void updateConfigFromRemote();

    const QString& storageLocked();

    const QString& credsUrl();
    const QString& xApiKey();
    const QString& url();

    QNetworkProxy* proxy();
    void clearProxy();

private:
    int proxyType();
    const QString& proxyHost();
    int proxyPort();
    const QString& proxyUser();
    const QString& proxyPassword();

    void initSettings();
    const QString& localConfigFilePath(const QString& fileName);
    void parseConfigurationData(const QString& data);
    void initS3Creds();
    void normalizeS3Creds();

    // class members
    QSettings* m_localSettings;
    QString m_storageLocked;
    QString m_qstr;

    // s3
    QUrl m_s3ConfigUrl;
    QString m_credsUrl;
    QString m_xApiKey;
    QString m_url;

    // proxy
    QNetworkProxy* m_proxy;
    QSettings* m_proxySettings;
    int m_proxyType;
    QString m_proxyHost;
    int m_proxyPort;
    QString m_proxyUser;
    QString m_proxyPassword;

    //
    QNetworkAccessManager* m_networkConfig;
};

#endif // IMG_S3_SETTINGS_H
