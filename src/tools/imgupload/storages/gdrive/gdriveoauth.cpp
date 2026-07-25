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
// Least-privilege Drive scope -- app-created files only (R7, KD3) -- plus the
// two OpenID Connect sign-in scopes. The identity scopes are what carry the
// account email and the `hd` (hosted domain) claim that org sharing needs
// (KTD6); Drive's own about.get cannot be relied on for the email under
// drive.file alone.
//
// The last two are read-only directory lookups that back recipient suggestion:
// `directory.readonly` prefix-searches people in the organization, and
// `cloud-identity.groups.readonly` reads the signed-in user's own group
// memberships. Neither can write anything. `directory.readonly` is classified
// sensitive, which is acceptable for an Internal-only app but is a change from
// the earlier all-non-sensitive scope set (see docs/google-drive-setup.md).
//
// This is the single place the requested scope string is written. The marker
// comparison in requestAccessToken() is an exact string match, so a second
// spelling anywhere would re-consent on every startup forever.
const char* kScope =
  "https://www.googleapis.com/auth/drive.file openid email "
  "https://www.googleapis.com/auth/directory.readonly "
  "https://www.googleapis.com/auth/cloud-identity.groups.readonly";
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

/**
 * Decode the claim set of a Google-issued ID token. Empty if the token is
 * absent or malformed.
 *
 * The signature is deliberately not verified. The token arrives in the body of
 * our own client-authenticated HTTPS POST to Google's token endpoint, which
 * Google's guidance treats as establishing origin for a token received directly
 * that way. Forging claims here would require owning that TLS channel -- which
 * hands over the access token in the same response, so a spoofed `hd` buys an
 * attacker nothing they could not already do with the token itself.
 */
QJsonObject idTokenClaims(const QString& idToken)
{
    // header.payload.signature; the claims are the unpadded base64url middle.
    const QList<QByteArray> segments = idToken.toLatin1().split('.');
    if (segments.size() != 3) {
        return {};
    }
    return QJsonDocument::fromJson(
             QByteArray::fromBase64(segments.at(1),
                                    QByteArray::Base64UrlEncoding))
      .object();
}
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
  , m_silentRefreshActive(false)
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

    ConfigHandler config;
    // A grant never gains a scope by being refreshed -- Google only widens one
    // through a fresh consent. A refresh token stored under a scope set other
    // than this build's must therefore be re-consented once, or the missing
    // scope stays missing forever. The marker records what was *requested*,
    // not what was granted, so a user who declines an optional scope at the
    // granular consent screen is not re-prompted on every upload.
    if (!config.gdriveRefreshToken().isEmpty() &&
        config.gdriveGrantedScopes() == QString::fromLatin1(kScope)) {
        refreshAccessToken();
    } else {
        startConsentFlow();
    }
}

void GDriveOAuth::requestAccessTokenSilently(
  std::function<void(const QString&)> done)
{
    if (!done) {
        return;
    }

    // Reuse a still-valid in-memory token. Once the session's first upload has
    // authorized, this is the branch every keystroke takes.
    if (!m_accessToken.isEmpty() &&
        m_accessTokenExpiry > QDateTime::currentDateTimeUtc()) {
        done(m_accessToken);
        return;
    }

    // Join a silent refresh already in flight instead of issuing a second one.
    if (m_silentRefreshActive) {
        m_silentWaiters.append(std::move(done));
        return;
    }

    // Every remaining failure branch reports "no token" rather than escalating.
    // This entry point must not reach startConsentFlow() on any path (KTD1):
    // the recipient field is live while the user types, and the upload dialog
    // is constructed before the upload that would otherwise have warmed the
    // token, so an escalating lookup would open a browser window mid-share.
    ConfigHandler config;
    if (config.gdriveClientId().isEmpty() ||
        config.gdriveClientSecret().isEmpty() ||
        config.gdriveRefreshToken().isEmpty() ||
        config.gdriveGrantedScopes() != QString::fromLatin1(kScope)) {
        // No usable grant, or one stored under a narrower scope set that cannot
        // carry the directory scopes. Only a fresh consent could widen it, and
        // that is the upload path's business, not a lookup's.
        done(QString());
        return;
    }
    if (m_flowActive) {
        // An interactive flow is running and its browser tab may sit open for
        // minutes. Report unavailable now rather than queueing behind it; if it
        // grants, the in-memory token above serves the next keystroke.
        done(QString());
        return;
    }

    m_silentWaiters.append(std::move(done));
    m_silentRefreshActive = true;

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

    // Deliberately does not set m_flowActive: that flag arbitrates the single
    // interactive flow, and a lookup must neither block an upload's consent nor
    // make an upload think a flow it can join is already running.
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
            resolveSilentWaiters(m_accessToken);
            return;
        }
        // Stored credentials are left untouched even on invalid_grant. A
        // suggestion lookup is a passenger on the upload path's grant; the next
        // upload runs the same refresh through requestAccessToken(), which owns
        // clearing a dead token and re-consenting.
        resolveSilentWaiters(QString());
    });
}

void GDriveOAuth::resolveSilentWaiters(const QString& token)
{
    m_silentRefreshActive = false;
    // Take the list first: a callback may request another token synchronously.
    QList<std::function<void(const QString&)>> waiters;
    waiters.swap(m_silentWaiters);
    for (const std::function<void(const QString&)>& waiter : waiters) {
        waiter(token);
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
    query.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
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
        connect(
          socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
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

            const QString error = query.queryItemValue(QStringLiteral("error"));
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
        m_codeVerifier.clear();

        // The ID token rides along in this same response, so the account email
        // and the organization domain cost no extra request and no guesswork:
        // `hd` is the account's Workspace domain verbatim. It is absent for a
        // personal Google account, which correctly means "no organization to
        // share with" rather than "lookup failed".
        const QJsonObject claims =
          idTokenClaims(json.value(QStringLiteral("id_token")).toString());
        const QString email = claims.value(QStringLiteral("email")).toString();
        const QString domain = claims.value(QStringLiteral("hd")).toString();

        ConfigHandler config;
        const QString refresh =
          json.value(QStringLiteral("refresh_token")).toString();
        if (!refresh.isEmpty()) {
            config.setGdriveRefreshToken(refresh);
            // Record the scope set this grant was requested under, so the next
            // scope change re-consents exactly once (see requestAccessToken).
            config.setGdriveGrantedScopes(QString::fromLatin1(kScope));
        }
        if (!email.isEmpty()) {
            // Fresh consent is authoritative about which account is connected,
            // including an empty domain for a personal account.
            config.setGdriveAccountEmail(email);
            config.setGdriveAccountDomain(domain);
        }
        // Flush before chmod: QSettings defers the write, and its later
        // QSaveFile rewrite would otherwise reset the file to umask
        // permissions after the chmod, exposing the refresh token.
        config.flush();
        reassertConfigPermissions(config.configFilePath());

        // A fresh consent always re-picks the account, so anything cached per
        // account is now suspect -- announce it before any lookup can run
        // against the new grant. Dropping a cache that happened to belong to
        // the same account again is harmless; keeping one that belonged to the
        // previous account is not.
        emit accountChanged();

        if (email.isEmpty()) {
            // No ID token, or the sign-in scopes were declined at the granular
            // consent screen. Fall back to about.get, which may still carry the
            // email; the upload itself does not depend on either.
            fetchAccountInfo();
            return;
        }
        finishGranted();
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
            // otherwise just clear the token and let the next upload restart
            // it, rather than surprising the user with an unsolicited browser
            // tab.
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
      "Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toUtf8());
    request.setTransferTimeout(kRequestTimeoutMs);

    QNetworkReply* reply = m_net->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            // Best-effort fallback: the token, not the account info, is what
            // gates the upload. Sharing degrades with an explicit warning.
            finishGranted();
            return;
        }
        const QJsonObject json =
          QJsonDocument::fromJson(reply->readAll()).object();
        const QString email = json.value(QStringLiteral("user"))
                                .toObject()
                                .value(QStringLiteral("emailAddress"))
                                .toString();
        if (!email.isEmpty()) {
            ConfigHandler config;
            config.setGdriveAccountEmail(email);
            // Only reachable when the ID token carried no email, so there is no
            // `hd` claim to prefer: fall back to the address's own domain.
            const int at = email.indexOf('@');
            if (at >= 0) {
                config.setGdriveAccountDomain(email.mid(at + 1));
            }
            config.flush();
            reassertConfigPermissions(config.configFilePath());
        }
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
    config.setGdriveGrantedScopes(QStringLiteral(""));
    config.setGdriveAccountEmail(QStringLiteral(""));
    config.setGdriveAccountDomain(QStringLiteral(""));
    config.setGdriveFolderId(QStringLiteral(""));
    m_accessToken.clear();
    m_accessTokenExpiry = QDateTime();
    config.flush();
    reassertConfigPermissions(config.configFilePath());
    emit accountChanged();
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
