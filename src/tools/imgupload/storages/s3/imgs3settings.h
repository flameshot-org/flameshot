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

#ifndef IMG_S3_SETTINGS_H
#define IMG_S3_SETTINGS_H

#define S3_API_IMG_PATH "v2/image/"

#define S3_REMOTE_CONFIG_URL                                                   \
    "https://git.namecheap.net/projects/RND/repos/flameshot_config/raw/teams/" \
    "wide-pr/config.ini"
#define S3_CONFIG_LOCAL "config.ini"
#define S3_CONFIG_PROXY "config_proxy.ini"

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariant>

class QSettings;
class QNetworkProxy;

class ImgS3Settings
{

public:
    ImgS3Settings();

    const QString& credsUrl();
    const QString& xApiKey();
    const QString& url();
    const QUrl& configUrl();

    QNetworkProxy* proxy();
    void clearProxy();

    void updateConfigurationData(const QString& data);

    void setValue(const QString& group,
                  const QString& key,
                  const QVariant& value);
    QVariant value(const QString& group, const QString& key) const;

private:
    int proxyType();
    const QString& proxyHost();
    int proxyPort();
    const QString& proxyUser();
    const QString& proxyPassword();

    void initSettings();
    const QString& localConfigFilePath(const QString& fileName);
    void initS3Creds();
    void normalizeS3Creds();
    void updateSettingsFromRemoteConfig(const QSettings* settings);

    // class members
    QSettings* m_localSettings;
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
};

#endif // IMG_S3_SETTINGS_H
