// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#include "gdriveoauth.h"
#include "utils/confighandler.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace {
// Google OAuth 2.0 endpoints (KTD2).
const char* kAuthEndpoint = "https://accounts.google.com/o/oauth2/v2/auth";
const char* kTokenEndpoint = "https://oauth2.googleapis.com/token";
const char* kRevokeEndpoint = "https://oauth2.googleapis.com/revoke";
const char* kAboutEndpoint = "https://www.googleapis.com/drive/v3/about";
// Least-privilege scope: app-created files only (R7, KD3).
const char* kScope = "https://www.googleapis.com/auth/drive.file";
constexpr int kConsentTimeoutMs = 180 * 1000; // ~3 minutes (KTD9)
constexpr int kRequestTimeoutMs = 30 * 1000;

// Fully static, self-contained response page. No request data is ever echoed
// into it (KTD9).
const char* kResponsePage =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/html; charset=utf-8\r\n"
  "Connection: close\r\n"
  "\r\n"
  "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
  "<title>Flameshot</title></head><body style=\"font-family:sans-serif\">"
  "<h2>Flameshot is now authorized.</h2>"
  "<p>You can close this tab and return to Flameshot.</p>"
  "</body></html>";
}

GDriveOAuth* GDriveOAuth::instance()
{
    static GDriveOAuth* self = new GDriveOAuth(qApp);
    return self;
}

GDriveOAuth::GDriveOAuth(QObject* parent)
  : QObject(parent)
  , m_net(new QNetworkAccessManager(this))
  , m_server(nullptr)
  , m_timeout(new QTimer(this))
  , m_flowActive(false)
  , m_waiters(0)
  , m_redirectPort(0)
{
    m_timeout->setSingleShot(true);
    connect(m_timeout, &QTimer::timeout, this, [this]() {
        finishFailed(tr("Authorization timed out. Please try again."));
    });
}

QString GDriveOAuth::accessToken() const
{
    return m_accessToken;
}

bool GDriveOAuth::isConnected() const
{
    return !ConfigHandler().gdriveRefreshToken().isEmpty();
}

QString GDriveOAuth::accountEmail() const
{
    return ConfigHandler().gdriveAccountEmail();
}

QString GDriveOAuth::accountDomain() const
{
    return ConfigHandler().gdriveAccountDomain();
}

void GDriveOAuth::attachWaiter()
{
    ++m_waiters;
}

void GDriveOAuth::detachWaiter()
{
    if (m_waiters > 0) {
        --m_waiters;
    }
    // The listener stops when the last waiter detaches -- but not on an
    // individual widget's destruction while others still wait (KTD9). Only tear
    // down during the browser-wait phase (listener active); an in-flight token
    // exchange/refresh (listener already closed) is left to complete so
    // m_flowActive stays set and a new request joins it rather than starting a
    // second, overlapping flow.
    if (m_waiters == 0 && m_flowActive && m_server != nullptr) {
        abortFlow();
    }
}

void GDriveOAuth::requestAccessToken()
{
    // Reuse a still-valid in-memory token.
    if (!m_accessToken.isEmpty() &&
        m_accessTokenExpiry > QDateTime::currentDateTimeUtc()) {
        emit accessTokenReady(m_accessToken);
        return;
    }

    // A consent/refresh flow is already running: join it (the caller is already
    // connected to the resolution signals). One flow at a time (R15).
    if (m_flowActive) {
        return;
    }

    if (!ConfigHandler().gdriveRefreshToken().isEmpty()) {
        refreshAccessToken();
    } else {
        startConsentFlow();
    }
}

void GDriveOAuth::startConsentFlow()
{
    ConfigHandler config;
    if (config.gdriveClientId().isEmpty() ||
        config.gdriveClientSecret().isEmpty()) {
        finishFailed(tr("Google Drive is not configured. Enter your OAuth "
                        "client ID and secret in the settings."));
        return;
    }

    // Preflight TLS: on builds without OpenSSL (optional on Windows) HTTPS
    // would fail mid-flow with an opaque error.
    if (!QSslSocket::supportsSsl()) {
        finishFailed(tr("TLS support is unavailable, so Flameshot cannot "
                        "contact Google securely. Install OpenSSL and try "
                        "again."));
        return;
    }

    // Defensively tear down any prior listener before starting a new one, so a
    // superseded flow can never leave an orphaned QTcpServer bound.
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
    m_server = new QTcpServer(this);
    if (!m_server->listen(QHostAddress::LocalHost, 0)) {
        m_server->deleteLater();
        m_server = nullptr;
        finishFailed(tr("Could not start the local authorization listener."));
        return;
    }
    m_redirectPort = m_server->serverPort();
    connect(m_server,
            &QTcpServer::newConnection,
            this,
            &GDriveOAuth::onLoopbackConnection);

    m_codeVerifier = QString::fromLatin1(randomUrlSafe(32));
    m_state = QString::fromLatin1(randomUrlSafe(16)); // >= 128-bit CSPRNG
    const QByteArray challenge =
      QCryptographicHash::hash(m_codeVerifier.toLatin1(),
                               QCryptographicHash::Sha256)
        .toBase64(QByteArray::Base64UrlEncoding |
                  QByteArray::OmitTrailingEquals);

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client_id"), config.gdriveClientId());
    query.addQueryItem(
      QStringLiteral("redirect_uri"),
      QStringLiteral("http://127.0.0.1:%1").arg(m_redirectPort));
    query.addQueryItem(QStringLiteral("response_type"),
                       QStringLiteral("code"));
    query.addQueryItem(QStringLiteral("scope"), QString::fromLatin1(kScope));
    query.addQueryItem(QStringLiteral("code_challenge"),
                       QString::fromLatin1(challenge));
    query.addQueryItem(QStringLiteral("code_challenge_method"),
                       QStringLiteral("S256"));
    query.addQueryItem(QStringLiteral("state"), m_state);
    query.addQueryItem(QStringLiteral("access_type"),
                       QStringLiteral("offline"));
    // Google reliably reissues a refresh token only on fresh consent (KTD2).
    query.addQueryItem(QStringLiteral("prompt"), QStringLiteral("consent"));

    QUrl authUrl(QString::fromLatin1(kAuthEndpoint));
    authUrl.setQuery(query);

    m_flowActive = true;
    m_timeout->start(kConsentTimeoutMs);

    if (!QDesktopServices::openUrl(authUrl)) {
        finishFailed(tr("Could not open the system browser for "
                        "authorization."));
    }
}

void GDriveOAuth::onLoopbackConnection()
{
    while (m_server && m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            const QByteArray data = socket->readAll();
            const int lineEnd = data.indexOf("\r\n");
            const QByteArray requestLine =
              lineEnd >= 0 ? data.left(lineEnd) : data;

            // Always answer so the browser tab does not hang, then close.
            socket->write(kResponsePage);
            socket->disconnectFromHost();

            // The reply is only buffered at this point -- QTcpSocket flushes it
            // from the event loop. nextPendingConnection() parented this socket
            // to the listener, so any teardown of m_server (below on success,
            // or from abortFlow() on timeout/cancel/a superseded flow) would
            // delete the socket with the reply still queued. The browser then
            // sees a closed connection with zero bytes and retries, only to
            // find the listener already gone: connection refused instead of the
            // success page. Take ownership here so the reply outlives the
            // listener; the disconnected handler above frees it once the flush
            // completes.
            socket->setParent(this);

            // Parse "GET <target> HTTP/1.1"; extract the query string.
            const QList<QByteArray> parts = requestLine.split(' ');
            if (parts.size() < 2) {
                return;
            }
            const QByteArray target = parts.at(1);
            const int q = target.indexOf('?');
            if (q < 0) {
                return; // favicon/prefetch probe: ignore, keep waiting
            }
            QUrlQuery query(QString::fromUtf8(target.mid(q + 1)));

            // Only a redirect whose state matches the pending flow is honored;
            // everything else is tolerated without disturbing the flow (KTD9).
            if (!m_flowActive ||
                query.queryItemValue(QStringLiteral("state")) != m_state) {
                return;
            }

            const QString error =
              query.queryItemValue(QStringLiteral("error"));
            if (!error.isEmpty()) {
                // User denial renders as a plain cancellation (KTD9).
                if (error == QStringLiteral("access_denied")) {
                    finishCanceled();
                } else {
                    finishFailed(tr("Authorization failed: %1").arg(error));
                }
                return;
            }

            const QString code =
              query.queryItemValue(QStringLiteral("code"), QUrl::FullyDecoded);
            if (code.isEmpty()) {
                return;
            }

            // Single-use state: stop the listener/timer and exchange the code.
            m_state.clear();
            m_timeout->stop();
            if (m_server) {
                m_server->close();
                m_server->deleteLater();
                m_server = nullptr;
            }
            exchangeAuthCode(code);
        });
    }
}

void GDriveOAuth::exchangeAuthCode(const QString& code)
{
    ConfigHandler config;
    QUrlQuery body;
    body.addQueryItem(QStringLiteral("code"), code);
    body.addQueryItem(QStringLiteral("client_id"), config.gdriveClientId());
    body.addQueryItem(QStringLiteral("client_secret"),
                      config.gdriveClientSecret());
    body.addQueryItem(
      QStringLiteral("redirect_uri"),
      QStringLiteral("http://127.0.0.1:%1").arg(m_redirectPort));
    body.addQueryItem(QStringLiteral("grant_type"),
                      QStringLiteral("authorization_code"));
    body.addQueryItem(QStringLiteral("code_verifier"), m_codeVerifier);

    QNetworkRequest request{ QUrl(QString::fromLatin1(kTokenEndpoint)) };
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));
    request.setTransferTimeout(kRequestTimeoutMs);

    QNetworkReply* reply =
      m_net->post(request, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QJsonObject json =
          QJsonDocument::fromJson(reply->readAll()).object();
        if (reply->error() != QNetworkReply::NoError ||
            !json.contains(QStringLiteral("access_token"))) {
            finishFailed(tr("Could not obtain a Google Drive access token."));
            return;
        }
        m_accessToken = json.value(QStringLiteral("access_token")).toString();
        m_accessTokenExpiry = QDateTime::currentDateTimeUtc().addSecs(
          json.value(QStringLiteral("expires_in")).toInt(3600) - 60);
        const QString refresh =
          json.value(QStringLiteral("refresh_token")).toString();
        if (!refresh.isEmpty()) {
            ConfigHandler config;
            config.setGdriveRefreshToken(refresh);
            // Flush before chmod: QSettings defers the write, and its later
            // QSaveFile rewrite would otherwise reset the file to umask
            // permissions after the chmod, exposing the refresh token.
            config.flush();
            reassertConfigPermissions(config.configFilePath());
        }
        m_codeVerifier.clear();
        // Fetch account info once at authorization, then resolve.
        fetchAccountInfo();
    });
}

void GDriveOAuth::refreshAccessToken()
{
    ConfigHandler config;
    QUrlQuery body;
    body.addQueryItem(QStringLiteral("client_id"), config.gdriveClientId());
    body.addQueryItem(QStringLiteral("client_secret"),
                      config.gdriveClientSecret());
    body.addQueryItem(QStringLiteral("refresh_token"),
                      config.gdriveRefreshToken());
    body.addQueryItem(QStringLiteral("grant_type"),
                      QStringLiteral("refresh_token"));

    QNetworkRequest request{ QUrl(QString::fromLatin1(kTokenEndpoint)) };
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/x-www-form-urlencoded"));
    request.setTransferTimeout(kRequestTimeoutMs);

    m_flowActive = true;
    QNetworkReply* reply =
      m_net->post(request, body.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QJsonObject json =
          QJsonDocument::fromJson(reply->readAll()).object();
        if (json.contains(QStringLiteral("access_token"))) {
            m_accessToken =
              json.value(QStringLiteral("access_token")).toString();
            m_accessTokenExpiry = QDateTime::currentDateTimeUtc().addSecs(
              json.value(QStringLiteral("expires_in")).toInt(3600) - 60);
            finishGranted();
            return;
        }
        // A refresh failure with invalid_grant is terminal: clear the stored
        // token and re-run consent (R4, KTD2).
        const QString err = json.value(QStringLiteral("error")).toString();
        if (err == QStringLiteral("invalid_grant")) {
            ConfigHandler().setGdriveRefreshToken(QStringLiteral(""));
            m_flowActive = false;
            // Only re-open interactive consent if a widget is still waiting;
            // otherwise just clear the token and let the next upload restart it,
            // rather than surprising the user with an unsolicited browser tab.
            if (m_waiters > 0) {
                startConsentFlow();
            }
        } else {
            finishFailed(tr("Could not refresh the Google Drive session."));
        }
    });
}

void GDriveOAuth::fetchAccountInfo()
{
    QUrl url(QString::fromLatin1(kAboutEndpoint));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("fields"), QStringLiteral("user"));
    url.setQuery(query);

    QNetworkRequest request{ url };
    request.setRawHeader(
      "Authorization",
      QStringLiteral("Bearer %1").arg(m_accessToken).toUtf8());
    request.setTransferTimeout(kRequestTimeoutMs);

    QNetworkReply* reply = m_net->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const QJsonObject json =
          QJsonDocument::fromJson(reply->readAll()).object();
        const QString email = json.value(QStringLiteral("user"))
                                .toObject()
                                .value(QStringLiteral("emailAddress"))
                                .toString();
        if (!email.isEmpty()) {
            ConfigHandler config;
            config.setGdriveAccountEmail(email);
            const int at = email.indexOf('@');
            if (at >= 0) {
                config.setGdriveAccountDomain(email.mid(at + 1));
            }
            config.flush();
            reassertConfigPermissions(config.configFilePath());
        }
        // Account info is best-effort; the token is what gates the upload.
        finishGranted();
    });
}

void GDriveOAuth::disconnectAccount()
{
    // Invalidate any in-flight consent first (clears state/verifier/listener),
    // so completing its still-open browser tab cannot silently re-persist a
    // token and undo the disconnect the user just performed (R16, KTD9).
    if (m_flowActive) {
        finishCanceled();
    }

    ConfigHandler config;
    const QString refresh = config.gdriveRefreshToken();
    if (!refresh.isEmpty()) {
        // Best-effort server-side revocation so a leaked token stops working.
        QUrlQuery body;
        body.addQueryItem(QStringLiteral("token"), refresh);
        QNetworkRequest request{ QUrl(QString::fromLatin1(kRevokeEndpoint)) };
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/x-www-form-urlencoded"));
        request.setTransferTimeout(kRequestTimeoutMs);
        QNetworkReply* reply =
          m_net->post(request, body.toString(QUrl::FullyEncoded).toUtf8());
        connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
    }
    config.setGdriveRefreshToken(QStringLiteral(""));
    config.setGdriveAccountEmail(QStringLiteral(""));
    config.setGdriveAccountDomain(QStringLiteral(""));
    config.setGdriveFolderId(QStringLiteral(""));
    m_accessToken.clear();
    m_accessTokenExpiry = QDateTime();
    config.flush();
    reassertConfigPermissions(config.configFilePath());
}

void GDriveOAuth::abortFlow()
{
    m_timeout->stop();
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
    m_state.clear();
    m_codeVerifier.clear();
    m_flowActive = false;
}

void GDriveOAuth::finishGranted()
{
    const QString token = m_accessToken;
    abortFlow();
    emit accessTokenReady(token);
}

void GDriveOAuth::finishCanceled()
{
    abortFlow();
    emit authCanceled();
}

void GDriveOAuth::finishFailed(const QString& error)
{
    abortFlow();
    emit authFailed(error);
}

void GDriveOAuth::reassertConfigPermissions(const QString& path) const
{
    // QSettings rewrites the whole file on each save, so owner-only permissions
    // must be re-asserted after every persist (rclone precedent; U4/U8).
    // setPermissions() no-ops harmlessly if the file does not exist, so no
    // (racy) existence pre-check is needed.
    if (!path.isEmpty()) {
        QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner);
    }
}

QByteArray GDriveOAuth::randomUrlSafe(int bytes)
{
    QByteArray raw(bytes, Qt::Uninitialized);
    QRandomGenerator* rng = QRandomGenerator::system();
    for (int i = 0; i < bytes; ++i) {
        raw[i] = static_cast<char>(rng->bounded(256));
    }
    return raw.toBase64(QByteArray::Base64UrlEncoding |
                        QByteArray::OmitTrailingEquals);
}
