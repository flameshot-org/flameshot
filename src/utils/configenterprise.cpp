#include "configenterprise.h"
#include <QDir>
#include <QSettings>
#include <QFileInfo>


ConfigEnterprise::ConfigEnterprise()
{
    // get enterprise settings
    m_settings = nullptr;
    QString configIniPath = QDir(QDir::currentPath()).filePath("config.ini");
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if(!(QFileInfo::exists(configIniPath) && QFileInfo(configIniPath).isFile())) {
        configIniPath = "/etc/flameshot/config.ini";
    }
#endif
    m_settings = new QSettings(configIniPath, QSettings::IniFormat);
}

QSettings *ConfigEnterprise::settings() {
    return m_settings;
}
