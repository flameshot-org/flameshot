#include "configenterprise.h"
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QString>
#include <QSettings>


ConfigEnterprise::ConfigEnterprise()
{
    // get enterprise settings
    m_settings = nullptr;
    QString configIniPath = QDir(QDir::currentPath()).filePath("config.ini");
    if(!(QFileInfo::exists(configIniPath) && QFileInfo(configIniPath).isFile())) {
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
        configIniPath = "/etc/flameshot/config.ini";
#elif defined(Q_OS_WIN)
        // calculate workdir for flameshot on startup if is not set yet
        QSettings bootUpSettings(
                    "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                    QSettings::NativeFormat);
        QFileInfo fi(bootUpSettings.value("Flameshot").toString());
        configIniPath = QDir(fi.absolutePath()).filePath("config.ini");
#endif
    }
    m_settings = new QSettings(configIniPath, QSettings::IniFormat);
}

QSettings *ConfigEnterprise::settings() {
    return m_settings;
}
