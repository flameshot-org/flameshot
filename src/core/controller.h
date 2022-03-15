// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/core/capturerequest.h"
#include <QObject>
#include <QPointer>
#include <QVersionNumber>
#include <functional>

class SystemTray;
class CaptureWidget;
class ConfigWindow;
class InfoWindow;
class CaptureLauncher;
class HistoryWidget;
class QNetworkAccessManager;
class QNetworkReply;
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
class QHotkey;
#endif
using lambda = std::function<void(void)>;

class Controller : public QObject
{
    Q_OBJECT

public:
    enum Origin
    {
        CLI,
        DAEMON
    };

    static Controller* instance();

public slots:
    void gui(const CaptureRequest& req = CaptureRequest::GRAPHICAL_MODE);
    void screen(CaptureRequest req, const int screenNumber = -1);
    void full(const CaptureRequest& req);
    void launcher();
    void config();

    void info();
    void history();

    QVersionNumber getVersion();

public:
    static void setOrigin(Origin origin);
    static Origin origin();
    void getLatestAvailableVersion();

signals:
    // TODO remove all parameters from captureTaken and update dependencies
    void captureTaken(QPixmap p, const QRect& selection);
    void captureFailed();
    void newVersionAvailable(QVersionNumber version);

public slots:
    void requestCapture(const CaptureRequest& request);

    // TODO move tray icon handling to FlameshotDaemon
    void initTrayIcon();
    void enableTrayIcon();
    void disableTrayIcon();

    void exportCapture(QPixmap p, QRect& selection, const CaptureRequest& req);
    void sendTrayNotification(
      const QString& text,
      const QString& title = QStringLiteral("Flameshot Info"),
      const int timeout = 5000);

public slots:
    void handleReplyCheckUpdates(QNetworkReply* reply);
    void checkForUpdates();

private:
    Controller();
    bool resolveAnyConfigErrors();

    // replace QTimer::singleShot introduced in Qt 5.4
    // the actual target Qt version is 5.3
    void doLater(int msec, QObject* receiver, lambda func);

    // class members
    QString m_appLatestUrl;
    QString m_appLatestVersion;
    bool m_showCheckAppUpdateStatus;
    static Origin m_origin;

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<CaptureLauncher> m_launcherWindow;
    QPointer<ConfigWindow> m_configWindow;
    QPointer<SystemTray> m_trayIcon;

    QNetworkAccessManager* m_networkCheckUpdates;
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    QHotkey* m_HotkeyScreenshotCapture;
    QHotkey* m_HotkeyScreenshotHistory;
#endif
    friend class SystemTray; // FIXME remove
};
