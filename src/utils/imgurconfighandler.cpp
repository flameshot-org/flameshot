#include "imgurconfighandler.h"

ImgurConfigHandler::ImgurConfigHandler()
{
    m_imgurConfig = new QSettings(
        QSettings::IniFormat,
        QSettings::UserScope,
        qApp->organizationName(),
        QStringLiteral("imgur")
    );
}

bool ImgurConfigHandler::isAuthorized() const
{
    if (!m_imgurConfig->value(QStringLiteral("Api/client_id")).toString().isEmpty() &&
        !m_imgurConfig->value(QStringLiteral("Api/client_secret")).toString().isEmpty() &&
        !getToken().isEmpty()) {
        return true;
    }

    return false;
}

QMap<QString, QVariant> ImgurConfigHandler::getToken() const
{
    if (m_imgurConfig->value(QStringLiteral("Token/access_token")).toString().isEmpty() ||
        m_imgurConfig->value(QStringLiteral("Token/expires_in")).toInt() <= 0 ||
        m_imgurConfig->value(QStringLiteral("Token/token_type")).toString().isEmpty() ||
        m_imgurConfig->value(QStringLiteral("Token/refresh_token")).toString().isEmpty() ||
        m_imgurConfig->value(QStringLiteral("Token/account_username")).toString().isEmpty() ||
        m_imgurConfig->value(QStringLiteral("Token/account_id")).toInt() <= 0) {
        return {};
    }

    return QMap<QString, QVariant> {
        {"access_token", m_imgurConfig->value("Token/access_token").toString()},
        {"expires_in", m_imgurConfig->value("Token/expires_in").toInt()},
        {"token_type", m_imgurConfig->value("Token/token_type").toString()},
        {"refresh_token", m_imgurConfig->value("Token/refresh_token").toString()},
        {"account_username", m_imgurConfig->value("Token/account_username").toString()},
        {"account_id", m_imgurConfig->value("Token/account_id").toInt()}
    };
}

void ImgurConfigHandler::setApiCredentials(const QString &clientId, const QString &clientSecret)
{
    m_imgurConfig->setValue(QStringLiteral("Api/client_id"), clientId);
    m_imgurConfig->setValue(QStringLiteral("Api/client_secret"), clientSecret);
}

void ImgurConfigHandler::setToken(QMap<QString, QVariant> &token)
{
    m_imgurConfig->setValue(QStringLiteral("Token/access_token"), token.value(QStringLiteral("access_token")).toString());
    m_imgurConfig->setValue(QStringLiteral("Token/expires_in"), token.value(QStringLiteral("expires_in")).toInt());
    m_imgurConfig->setValue(QStringLiteral("Token/token_type"), token.value(QStringLiteral("token_type")).toString());
    m_imgurConfig->setValue(QStringLiteral("Token/refresh_token"), token.value(QStringLiteral("refresh_token")).toString());
    m_imgurConfig->setValue(QStringLiteral("Token/account_username"), token.value(QStringLiteral("account_username")).toString());
    m_imgurConfig->setValue(QStringLiteral("Token/account_id"), token.value(QStringLiteral("account_id")).toInt());
}

void ImgurConfigHandler::setSetting(const QString &key, const QVariant &value)
{
    m_imgurConfig->setValue(key, value);
}

QVariant ImgurConfigHandler::getSetting(const QString &key, const QVariant &defaultValue) const
{
    return m_imgurConfig->value(key, defaultValue);
}
