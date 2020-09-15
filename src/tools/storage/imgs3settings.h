#ifndef IMGS3SETTINGS_H
#define IMGS3SETTINGS_H

#define S3_API_IMG_PATH "v2/image/"


#include <QString>

class ConfigEnterprise;

class ImgS3Settings
{
public:
    ImgS3Settings();

    const QString &credsUrl();
    const QString &xApiKey();
    const QString &url();

private:
    ConfigEnterprise *m_configEnterprise;
    QString m_credsUrl;
    QString m_xApiKey;
    QString m_url;
};

#endif // IMGS3SETTINGS_H
