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
    static Controller* getInstance();

    Controller(const Controller&) = delete;
    ~Controller();
    void operator=(const Controller&) = delete;

    void enableExports();
    void updateRecentScreenshots();

    void setCheckForUpdatesEnabled(const bool enabled);

signals:
    void captureTaken(uint id, QPixmap p, QRect selection);
    void captureFailed(uint id);
    void captureSaved(uint id, QString savePath);

public slots:
    void requestCapture(const CaptureRequest& request);

    void openConfigWindow();
    void openInfoWindow();
    void appUpdates();
    void openLauncherWindow();
    void enableTrayIcon();
    void disableTrayIcon();
    void sendTrayNotification(
      const QString& text,
      const QString& title = QStringLiteral("Flameshot Info"),
      const int timeout = 5000);

    void updateConfigComponents();

    void showRecentScreenshots();

    void sendCaptureSaved(uint id, const QString& savePath);

private slots:
    void startFullscreenCapture(const uint id = 0);
    void startVisualCapture(const uint id = 0,
                            const QString& forcedSavePath = QString());
    void startScreenGrab(const uint id = 0, const int screenNumber = -1);

    void handleCaptureTaken(uint id, QPixmap p, QRect selection);
    void handleCaptureFailed(uint id);

    void handleReplyCheckUpdates(QNetworkReply* reply);

private:
    Controller();
    void getLatestAvailableVersion();

    // replace QTimer::singleShot introduced in Qt 5.4
    // the actual target Qt version is 5.3
    void doLater(int msec, QObject* receiver, lambda func);

    // class members
    QAction* m_appUpdates;
    QString m_appLatestUrl;
    QString m_appLatestVersion;
    bool m_showCheckAppUpdateStatus;

    QMap<uint, CaptureRequest> m_requestMap;
    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<CaptureLauncher> m_launcherWindow;
    QPointer<ConfigWindow> m_configWindow;
    QPointer<QSystemTrayIcon> m_trayIcon;

    HistoryWidget* m_history;
    QMenu* m_trayIconMenu;

    QNetworkAccessManager* m_networkCheckUpdates;
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    QHotkey* m_HotkeyScreenshotCapture;
    QHotkey* m_HotkeyScreenshotHistory;
#endif
};
