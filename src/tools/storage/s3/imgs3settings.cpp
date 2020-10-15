#include "imgs3settings.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>

ImgS3Settings::ImgS3Settings()
{
    initSettings();

    // get s3 credentials
    m_settings->beginGroup("S3");
    m_credsUrl = m_settings->value("S3_CREDS_URL").toString();
    m_credsUrl =
      m_credsUrl +
      ((m_credsUrl.length() > 0 && m_credsUrl[m_credsUrl.length() - 1] == '/')
         ? ""
         : "/") +
      S3_API_IMG_PATH;

    m_xApiKey = m_settings->value("S3_X_API_KEY").toString();

    m_url = m_settings->value("S3_URL").toString();
    m_url =
      m_url +
      ((m_url.length() > 0 && m_url[m_url.length() - 1] == '/') ? "" : "/");

    m_settings->endGroup();
}

QSettings* ImgS3Settings::settings()
{
    return m_settings;
}

void ImgS3Settings::initSettings()
{
    // get s3 settings
    QString configIniPath = QDir(QDir::currentPath()).filePath("config.ini");
    if (!(QFileInfo::exists(configIniPath) &&
          QFileInfo(configIniPath).isFile())) {
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
        configIniPath = "/etc/flameshot/config.ini";
#elif defined(Q_OS_WIN)
        // calculate workdir for flameshot on startup if is not set yet
        QSettings bootUpSettings(
          "HKEY_CURRENT_"
          "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
          QSettings::NativeFormat);
        QFileInfo fi(bootUpSettings.value("Flameshot").toString());
        configIniPath = QDir(fi.absolutePath()).filePath("config.ini");
#endif
    }
    m_settings = new QSettings(configIniPath, QSettings::IniFormat);
}

const QString& ImgS3Settings::credsUrl()
{
    return m_credsUrl;
}

const QString& ImgS3Settings::xApiKey()
{
    return m_xApiKey;
}

const QString& ImgS3Settings::url()
{
    return m_url;
}
