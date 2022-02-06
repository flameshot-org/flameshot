// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/core/capturerequest.h"
#include <QMap>
#include <QMenu>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QTimer>
#include <functional>

class CaptureWidget;
class ConfigWindow;
class InfoWindow;
class QSystemTrayIcon;
class CaptureLauncher;
class UploadHistory;
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

    static Controller* getInstance();

    void setCheckForUpdatesEnabled(const bool enabled);
    static void setOrigin(Origin origin);
    static Origin origin();

signals:
    // TODO remove all parameters from captureTaken and update dependencies
    void captureTaken(QPixmap p, const QRect& selection);
    void captureFailed();

public slots:
    void requestCapture(const CaptureRequest& request);

    void openConfigWindow();
    void openInfoWindow();
    void appUpdates();
    void openLauncherWindow();
    // TODO move tray icon handling to FlameshotDaemon
    void initTrayIcon();
    void enableTrayIcon();
    void disableTrayIcon();

    void showRecentUploads();

    void exportCapture(QPixmap p, QRect& selection, const CaptureRequest& req);
    void sendTrayNotification(
      const QString& text,
      const QString& title = QStringLiteral("Flameshot Info"),
      const int timeout = 5000);

private slots:
    void startFullscreenCapture(const CaptureRequest& req);
    void startVisualCapture(
      const CaptureRequest& req = CaptureRequest::GRAPHICAL_MODE);
    void startScreenGrab(CaptureRequest req, const int screenNumber = -1);

public slots: // TODO move these up
    void handleCaptureTaken(const CaptureRequest& req,
                            QPixmap p,
                            QRect selection);
    void handleCaptureFailed();

    void handleReplyCheckUpdates(QNetworkReply* reply);

private:
    Controller();
    ~Controller();
    void getLatestAvailableVersion();
    bool resolveAnyConfigErrors();

    // replace QTimer::singleShot introduced in Qt 5.4
    // the actual target Qt version is 5.3
    void doLater(int msec, QObject* receiver, lambda func);

    // class members
    QAction* m_appUpdates;
    QString m_appLatestUrl;
    QString m_appLatestVersion;
    bool m_showCheckAppUpdateStatus;
    static Origin m_origin;

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<CaptureLauncher> m_launcherWindow;
    QPointer<ConfigWindow> m_configWindow;
    QPointer<QSystemTrayIcon> m_trayIcon;

    QMenu* m_trayIconMenu;

    QNetworkAccessManager* m_networkCheckUpdates;
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    QHotkey* m_HotkeyScreenshotCapture;
    QHotkey* m_HotkeyScreenshotHistory;
#endif
};
