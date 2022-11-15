// Copyright(c) 2017-2021 Namecheap inc.
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "imgs3settings.h"
#include "src/core/flameshot.h"
#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QNetworkProxy>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>

ImgS3Settings::ImgS3Settings()
  : m_proxy(nullptr)
  , m_localSettings(nullptr)
{
    initSettings();

    // get remote config url
    if (m_localSettings->contains("STORAGE_CONFIG_URL")) {
        m_s3ConfigUrl =
          QUrl(m_localSettings->value("STORAGE_CONFIG_URL").toString());
    } else {
        // set default value if STORAGE_CONFIG_URL not found in the config.ini
        m_s3ConfigUrl = QUrl(S3_REMOTE_CONFIG_URL);
    }

    // proxy settings
    m_proxyType = -1;
    m_proxyHost = QString();
    m_proxyPort = -1;
    m_proxyUser = QString();
    m_proxyPassword = QString();
}

void ImgS3Settings::initSettings()
{
    QString configIniPath =
      QFile::exists(S3_CONFIG_LOCAL)
        ? QDir::currentPath() + QDir::separator() + S3_CONFIG_LOCAL
        : QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
            QDir::separator() + "flameshot" + QDir::separator() +
            S3_CONFIG_LOCAL;
#if defined(Q_OS_MACOS)
    if (!QFile::exists(configIniPath)) {
        configIniPath =
          QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
          QDir::separator() + ".config" + QDir::separator() + "flameshot" +
          QDir::separator() + S3_CONFIG_LOCAL;
    }
#elif defined(Q_OS_WIN)
    if (!(QFileInfo::exists(configIniPath) &&
          QFileInfo(configIniPath).isFile())) {
        configIniPath =
          QDir("C:\\Program Files\\Flameshot\\bin").filePath(S3_CONFIG_LOCAL);
    }
#endif

    if (QFile::exists(configIniPath)) {
        m_localSettings = new QSettings(localConfigFilePath(configIniPath),
                                        QSettings::IniFormat);
        updateSettingsFromRemoteConfig(m_localSettings);
    } else {
        m_localSettings = new QSettings();
    }
    m_proxySettings =
      new QSettings(localConfigFilePath(S3_CONFIG_PROXY), QSettings::IniFormat);
}

void ImgS3Settings::initS3Creds()
{
    m_credsUrl = value("S3", "S3_CREDS_URL").toString();
    m_xApiKey = value("S3", "S3_X_API_KEY").toString();
    m_url = value("S3", "S3_URL").toString();
    normalizeS3Creds();
}

void ImgS3Settings::updateConfigurationData(const QString& data)
{
    // read remote and save to the temporary file
    QTemporaryFile file;
    file.open();
    QTextStream stream(&file);
    stream << data;
    stream.flush();

    // parse and get configuration data
    QSettings remoteConfig(file.fileName(), QSettings::IniFormat);
    remoteConfig.beginGroup("S3");
    QString url = remoteConfig.value("S3_URL").toString();
    QString credsUrl = remoteConfig.value("S3_CREDS_URL").toString();
    QString xApiKey = remoteConfig.value("S3_X_API_KEY").toString();
    normalizeS3Creds();
    remoteConfig.endGroup();

    // close and remove temporary file
    file.close();

    // update fixed settings
    updateSettingsFromRemoteConfig(&remoteConfig);

    // cache configuration at the local storage
    setValue("S3", "S3_URL", url);
    setValue("S3", "S3_CREDS_URL", credsUrl);
    setValue("S3", "S3_X_API_KEY", xApiKey);

    // set last update date
    QString currentDateTime =
      QDateTime::currentDateTime().toString(Qt::ISODate);
    setValue("S3", "S3_CREDS_UPDATED", QVariant(currentDateTime));
}

void ImgS3Settings::normalizeS3Creds()
{
    if (!m_url.isEmpty() && m_url.right(1) != "/") {
        m_url += "/";
    }
    if (!m_credsUrl.isEmpty() && m_credsUrl.right(1) != "/") {
        m_credsUrl += "/";
    }
}

void ImgS3Settings::updateSettingsFromRemoteConfig(const QSettings* settings)
{
    // TODO - check if this one is still required
    //    if (settings->contains("checkForUpdates")) {
    //        bool checkForUpdates =
    //        settings->value("checkForUpdates").toBool();
    //        ConfigHandler().setCheckForUpdates(checkForUpdates);
    //        Controller::getInstance()->setCheckForUpdatesEnabled(checkForUpdates);
    //    }
}

const QString& ImgS3Settings::localConfigFilePath(const QString& fileName)
{
    // get s3 settings
    m_qstr = QDir(QDir::currentPath()).filePath(fileName);
    if (!(QFileInfo::exists(m_qstr) && QFileInfo(m_qstr).isFile())) {
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
        m_qstr = "/etc/flameshot/" + fileName;
#elif defined(Q_OS_WIN)
        // calculate workdir for flameshot on startup if is not set yet
        QSettings bootUpSettings(
          "HKEY_CURRENT_"
          "USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
          QSettings::NativeFormat);
        QFileInfo fi(bootUpSettings.value("Flameshot").toString());
        m_qstr = QDir(fi.absolutePath()).filePath(fileName);
#endif
    }
    return m_qstr;
}

const QString& ImgS3Settings::credsUrl()
{
    if (m_credsUrl.isEmpty()) {
        initS3Creds();
        if (!m_credsUrl.isEmpty()) {
            m_credsUrl += S3_API_IMG_PATH;
        }
    }
    return m_credsUrl;
}

const QString& ImgS3Settings::xApiKey()
{
    if (m_xApiKey.isEmpty()) {
        initS3Creds();
    }
    return m_xApiKey;
}

const QString& ImgS3Settings::url()
{
    if (m_url.isEmpty()) {
        initS3Creds();
    }
    return m_url;
}

QNetworkProxy* ImgS3Settings::proxy()
{
    if (proxyHost().length() > 0) {
        m_proxy = new QNetworkProxy();
        switch (proxyType()) {
            case 0:
                m_proxy->setType(QNetworkProxy::DefaultProxy);
                break;
            case 1:
                m_proxy->setType(QNetworkProxy::Socks5Proxy);
                break;
            case 2:
                m_proxy->setType(QNetworkProxy::NoProxy);
                break;
            case 4:
                m_proxy->setType(QNetworkProxy::HttpCachingProxy);
                break;
            case 5:
                m_proxy->setType(QNetworkProxy::FtpCachingProxy);
                break;
            case 3:
            default:
                m_proxy->setType(QNetworkProxy::HttpProxy);
                break;
        }
        m_proxy->setHostName(proxyHost());
        m_proxy->setPort(proxyPort());
        if (proxyUser().length() > 0) {
            m_proxy->setUser(proxyUser());
            m_proxy->setPassword(proxyPassword());
        }

    } else {
        // Get proxy settings from OS settings
        QNetworkProxyQuery q(QUrl(credsUrl().toUtf8()));
        q.setQueryType(QNetworkProxyQuery::UrlRequest);
        q.setProtocolTag("http");

        QList<QNetworkProxy> proxies =
          QNetworkProxyFactory::systemProxyForQuery(q);
        if (proxies.size() > 0 && proxies[0].type() != QNetworkProxy::NoProxy) {
            m_proxy = new QNetworkProxy();
            m_proxy->setHostName(proxies[0].hostName());
            m_proxy->setPort(proxies[0].port());
            m_proxy->setType(proxies[0].type());
            m_proxy->setUser(proxies[0].user());
            m_proxy->setPassword(proxies[0].password());
        }
    }
    return m_proxy;
}

void ImgS3Settings::clearProxy()
{
    if (m_proxy != nullptr) {
        delete m_proxy;
        m_proxy = nullptr;
    }
}

int ImgS3Settings::proxyType()
{
    if (-1 == m_proxyType) {
        m_proxyType = 3; // default - HTTP transparent proxying is used
        if (m_proxySettings->contains("HTTP_PROXY_TYPE")) {
            m_proxyType = m_proxySettings->value("HTTP_PROXY_TYPE").toInt();
            if (m_proxyType < 0 || m_proxyType > 5) {
                m_proxyType = 3; // default - HTTP transparent proxying is used
            }
        }
    }
    return m_proxyType;
}

const QString& ImgS3Settings::proxyHost()
{
    if (m_proxyHost.isNull()) {
        if (m_proxySettings->contains("HTTP_PROXY_HOST")) {
            m_proxyHost = m_proxySettings->value("HTTP_PROXY_HOST").toString();
        } else {
            m_proxyHost = "";
        }
    }
    return m_proxyHost;
}

int ImgS3Settings::proxyPort()
{
    if (-1 == m_proxyPort) {
        m_proxyPort = 3128;
        if (m_proxySettings->contains("HTTP_PROXY_PORT")) {
            m_proxyPort = m_proxySettings->value("HTTP_PROXY_PORT").toInt();
        }
    }
    return m_proxyPort;
}

const QString& ImgS3Settings::proxyUser()
{
    if (m_proxyUser.isNull()) {
        if (m_proxySettings->contains("HTTP_PROXY_USER")) {
            m_proxyUser = m_proxySettings->value("HTTP_PROXY_USER").toString();
        } else {
            m_proxyUser = "";
        }
    }
    return m_proxyUser;
}

const QString& ImgS3Settings::proxyPassword()
{
    if (m_proxyPassword.isNull()) {
        if (m_proxySettings->contains("HTTP_PROXY_PASSWORD")) {
            m_proxyPassword =
              m_proxySettings->value("HTTP_PROXY_PASSWORD").toString();
        } else {
            m_proxyPassword = "";
        }
    }
    return m_proxyPassword;
}

const QUrl& ImgS3Settings::configUrl()
{
    return m_s3ConfigUrl;
}

void ImgS3Settings::setValue(const QString& group,
                             const QString& key,
                             const QVariant& value)
{
    m_localSettings->beginGroup(group);
    m_localSettings->setValue(key, value);
    m_localSettings->endGroup();
}

QVariant ImgS3Settings::value(const QString& group, const QString& key) const
{
    m_localSettings->beginGroup(group);
    QVariant qvar = m_localSettings->value(key);
    m_localSettings->endGroup();
    return qvar;
}
