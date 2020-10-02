#ifndef IMGS3SETTINGS_H
#define IMGS3SETTINGS_H

#define S3_API_IMG_PATH "v2/image/"

#include <QString>

class QSettings;

class ImgS3Settings
{
public:
    ImgS3Settings();

    const QString& credsUrl();
    const QString& xApiKey();
    const QString& url();
    QSettings* settings();

private:
    void initSettings();

    // class members
    QSettings* m_settings;
    QString m_credsUrl;
    QString m_xApiKey;
    QString m_url;
};

#endif // IMGS3SETTINGS_H
