// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/core/capturerequest.h"
#include <QObject>
#include <QPointer>
#include <QVersionNumber>

class CaptureWidget;
class ConfigWindow;
class InfoWindow;
class CaptureLauncher;
#ifdef ENABLE_IMGUR
class UploadHistory;
#endif
#if (defined(Q_OS_MAC) || defined(Q_OS_MACOS))
class QHotkey;
#endif

enum ErrCode : uint8_t
{
    E_OK = 0,
    E_GENERAL,
    E_ABORTED,
    E_DBUSCONN,
    E_SIG_BASE = 128,
    E_SIGINT = E_SIG_BASE + 2,
    E_SIGTERM = E_SIG_BASE + 15,
};

class Flameshot : public QObject
{
    Q_OBJECT

public:
    enum Origin
    {
        CLI,
        DAEMON
    };

    static Flameshot* instance();

public slots:
    CaptureWidget* gui(
      const CaptureRequest& req = CaptureRequest::GRAPHICAL_MODE);
    void screen(CaptureRequest req, int const screenNumber = -1);
    void full(const CaptureRequest& req);
    void launcher();
    void config();

    void info();

#ifdef ENABLE_IMGUR
    void history();
#endif

    void openSavePath();

    QVersionNumber getVersion();

public:
    static void setOrigin(Origin origin);
    static Origin origin();
    void setExternalWidget(bool b);
    bool haveExternalWidget();

signals:
    void captureTaken(QPixmap p);
    void captureFailed();

public slots:
    void requestCapture(const CaptureRequest& request);
    void exportCapture(const QPixmap& p,
                       QRect& selection,
                       const CaptureRequest& req);

private:
    Flameshot();
    bool resolveAnyConfigErrors();

    // class members
    static Origin m_origin;
    bool m_haveExternalWidget;

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<CaptureLauncher> m_launcherWindow;
    QPointer<ConfigWindow> m_configWindow;

#if (defined(Q_OS_MAC) || defined(Q_OS_MACOS))
    QHotkey* m_HotkeyScreenshotCapture;
    QHotkey* m_HotkeyScreenshotHistory;
#endif
};
