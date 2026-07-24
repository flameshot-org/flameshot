// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Flameshot Contributors

#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>

class QTcpServer;
class QTcpSocket;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

/**
 * Process-wide, application-lifetime OAuth 2.0 service for the Google Drive
 * backend (KTD1, KTD2, KTD9).
 *
 * Every capture spawns an independent, self-deleting uploader widget, so per
 * widget auth state cannot satisfy the single-flight requirement (R15). This
 * service instead owns the loopback listener and the token store for the whole
 * process; uploader widgets attach as waiters over its signals and detach when
 * they are destroyed. Only one interactive consent flow runs at a time; uploads
 * arriving while consent is pending subscribe to the same flow's outcome.
 *
 * The loopback listener binds strictly to 127.0.0.1 on an OS-assigned port and
 * tolerates unrelated traffic: only a redirect whose `state` matches the
 * pending flow completes authorization. It stops on a valid redirect, denial,
 * the timeout, or when the last waiter detaches -- never on an individual
 * widget's destruction.
 *
 * Authorization codes, tokens, and PKCE verifiers are never written to logs.
 */
class GDriveOAuth : public QObject
{
    Q_OBJECT
public:
    static GDriveOAuth* instance();

    /** A currently-valid in-memory access token, or empty if none. */
    QString accessToken() const;

    /**
     * Ensure a valid access token is available, running whatever is needed
     * (in-memory reuse, refresh, or interactive consent). Resolves through
     * exactly one of the signals below. Safe to call from several widgets at
     * once: additional callers join the in-flight flow instead of starting a
     * second browser consent.
     *
     * Callers must attachWaiter() before calling and detachWaiter() when they
     * stop caring about the outcome (typically on destruction).
     */
    void requestAccessToken();

    /** Register/unregister interest in a pending flow (listener lifetime). */
    void attachWaiter();
    void detachWaiter();

    bool isConnected() const;         // a refresh token is stored
    QString accountEmail() const;     // cached signed-in account
    QString accountDomain() const;    // cached org domain (for domain sharing)

    /**
     * Clear stored credentials and make a best-effort server-side revocation of
     * the refresh token. The next upload re-initiates consent (R16).
     */
    void disconnectAccount();

signals:
    void accessTokenReady(const QString& token);
    void authCanceled();
    void authFailed(const QString& error);

private:
    explicit GDriveOAuth(QObject* parent = nullptr);

    void startConsentFlow();
    void refreshAccessToken();
    void exchangeAuthCode(const QString& code);
    void fetchAccountInfo(); // about.get -> email + domain, cached
    void onLoopbackConnection();
    void abortFlow();        // stop listener + timer without emitting
    void finishGranted();
    void finishCanceled();
    void finishFailed(const QString& error);
    void reassertConfigPermissions(const QString& path) const;

    static QByteArray randomUrlSafe(int bytes);

    QNetworkAccessManager* m_net;
    QTcpServer* m_server;
    QTimer* m_timeout;

    // Pending-flow state (valid only while m_flowActive).
    bool m_flowActive;
    int m_waiters;
    quint16 m_redirectPort;
    QString m_state;         // single-use CSRF token for the pending flow
    QString m_codeVerifier;  // PKCE verifier for the pending flow

    // In-memory access token.
    QString m_accessToken;
    QDateTime m_accessTokenExpiry;
};
