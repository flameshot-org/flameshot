#include "imgs3settings.h"
#include "src/utils/configenterprise.h"
#include <QSettings>

ImgS3Settings::ImgS3Settings()
{
    m_configEnterprise = new ConfigEnterprise();

    // get s3 credentials
    QSettings *settings = m_configEnterprise->settings();
    settings->beginGroup("S3");

    m_credsUrl = settings->value("S3_CREDS_URL").toString();
    m_credsUrl = m_credsUrl + (m_credsUrl.length() > 0 && m_credsUrl.at(m_credsUrl.length() - 1) == "/" ? "" : "/") + S3_API_IMG_PATH;

    m_xApiKey = settings->value("S3_X_API_KEY").toString();

    m_url = settings->value("S3_URL").toString();
    m_url = m_url + (m_url.length() > 0 && m_url.at(m_url.length() - 1) == "/" ? "" : "/");

    settings->endGroup();

}

const QString &ImgS3Settings::credsUrl() {
    return m_credsUrl;
}

const QString &ImgS3Settings::xApiKey() {
    return m_xApiKey;
}

const QString &ImgS3Settings::url() {
    return m_url;
}
