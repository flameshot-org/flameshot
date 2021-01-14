// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "controller.h"
#include "src/config/configwindow.h"
#include "src/utils/confighandler.h"
#include "src/utils/history.h"
#include "src/utils/screengrabber.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/capture/capturetoolbutton.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/widgets/capturelauncher.h"
#include "src/widgets/historywidget.h"
#include "src/widgets/infowindow.h"
#include "src/widgets/notificationwidget.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QMenu>
#include <QSystemTrayIcon>

#ifdef Q_OS_WIN
#include "src/core/globalshortcutfilter.h"
#endif

// Controller is the core component of Flameshot, creates the trayIcon and
// launches the capture widget

Controller::Controller()
  : m_captureWindow(nullptr)
{
    m_history = nullptr;

    qApp->setQuitOnLastWindowClosed(false);

    // set default shortcusts if not set yet
    ConfigHandler().setShortcutsDefault();

    // init tray icon
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (!ConfigHandler().disabledTrayIconValue()) {
        enableTrayIcon();
    }
#elif defined(Q_OS_WIN)
    enableTrayIcon();

    GlobalShortcutFilter* nativeFilter = new GlobalShortcutFilter(this);
    qApp->installNativeEventFilter(nativeFilter);
    connect(nativeFilter, &GlobalShortcutFilter::printPressed, this, [this]() {
        this->requestCapture(CaptureRequest(CaptureRequest::GRAPHICAL_MODE));
    });
#endif

    QString StyleSheet = CaptureButton::globalStyleSheet();
    qApp->setStyleSheet(StyleSheet);
}

Controller::~Controller()
{
    delete m_history;
}

Controller* Controller::getInstance()
{
    static Controller c;
    return &c;
}

void Controller::enableExports()
{
    connect(
      this, &Controller::captureTaken, this, &Controller::handleCaptureTaken);
    connect(
      this, &Controller::captureFailed, this, &Controller::handleCaptureFailed);
}

void Controller::requestCapture(const CaptureRequest& request)
{
    uint id = request.id();
    m_requestMap.insert(id, request);

    switch (request.captureMode()) {
        case CaptureRequest::FULLSCREEN_MODE:
            doLater(request.delay(), this, [this, id]() {
                this->startFullscreenCapture(id);
            });
            break;
        // TODO: Figure out the code path that gets here so the deprated warning
        // can be fixed
        case CaptureRequest::SCREEN_MODE: {
            int&& number = request.data().toInt();
            doLater(request.delay(), this, [this, id, number]() {
                this->startScreenGrab(id, number);
            });
            break;
        }
        case CaptureRequest::GRAPHICAL_MODE: {
            QString&& path = request.path();
            doLater(request.delay(), this, [this, id, path]() {
                this->startVisualCapture(id, path);
            });
            break;
        }
        default:
            emit captureFailed(id);
            break;
    }
}

// creation of a new capture in GUI mode
void Controller::startVisualCapture(const uint id,
                                    const QString& forcedSavePath)
{
    if (!m_captureWindow) {
        QWidget* modalWidget = nullptr;
        do {
            modalWidget = qApp->activeModalWidget();
            if (modalWidget) {
                modalWidget->close();
                modalWidget->deleteLater();
            }
        } while (modalWidget);

        m_captureWindow = new CaptureWidget(id, forcedSavePath);
        // m_captureWindow = new CaptureWidget(id, forcedSavePath, false); //
        // debug
        connect(m_captureWindow,
                &CaptureWidget::captureFailed,
                this,
                &Controller::captureFailed);
        connect(m_captureWindow,
                &CaptureWidget::captureTaken,
                this,
                &Controller::captureTaken);

#ifdef Q_OS_WIN
        m_captureWindow->show();
#else
        m_captureWindow->showFullScreen();
        // m_captureWindow->show(); // Debug
#endif
    } else {
        emit captureFailed(id);
    }
}

void Controller::startScreenGrab(const uint id, const int screenNumber)
{
    bool ok = true;
    int n = screenNumber;

    if (n < 0) {
        QPoint globalCursorPos = QCursor::pos();
        n = qApp->desktop()->screenNumber(globalCursorPos);
    }
    QPixmap p(ScreenGrabber().grabScreen(n, ok));
    if (ok) {
        QRect selection; // `flameshot screen` does not support --selection
        emit captureTaken(id, p, selection);
    } else {
        emit captureFailed(id);
    }
}

// creation of the configuration window
void Controller::openConfigWindow()
{
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
        m_configWindow->show();
    }
}

// creation of the window of information
void Controller::openInfoWindow()
{
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
    }
}

void Controller::openLauncherWindow()
{
    if (!m_launcherWindow) {
        m_launcherWindow = new CaptureLauncher();
    }
    m_launcherWindow->show();
}

void Controller::enableTrayIcon()
{
    if (m_trayIcon) {
        return;
    }
    QMenu* trayIconMenu = new QMenu();

    ConfigHandler().setDisabledTrayIcon(false);
    QAction* captureAction = new QAction(tr("&Take Screenshot"), this);
    connect(captureAction, &QAction::triggered, this, [this]() {
        // Wait 400 ms to hide the QMenu
        doLater(400, this, [this]() { this->startVisualCapture(); });
    });
    QAction* launcherAction = new QAction(tr("&Open Launcher"), this);
    connect(launcherAction,
            &QAction::triggered,
            this,
            &Controller::openLauncherWindow);
    QAction* configAction = new QAction(tr("&Configuration"), this);
    connect(
      configAction, &QAction::triggered, this, &Controller::openConfigWindow);
    QAction* infoAction = new QAction(tr("&About"), this);
    connect(infoAction, &QAction::triggered, this, &Controller::openInfoWindow);
    QAction* quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    // recent screenshots
    QAction* recentAction = new QAction(tr("&Latest Uploads"), this);
    connect(
      recentAction, SIGNAL(triggered()), this, SLOT(showRecentScreenshots()));

    // generate menu
    trayIconMenu->addAction(captureAction);
    trayIconMenu->addAction(launcherAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(recentAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(configAction);
    trayIconMenu->addAction(infoAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon();
    m_trayIcon->setToolTip(QStringLiteral("Flameshot"));
    m_trayIcon->setContextMenu(trayIconMenu);
    QIcon trayicon =
      QIcon::fromTheme("flameshot-tray", QIcon(":img/app/flameshot.png"));
    m_trayIcon->setIcon(trayicon);

    auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::Trigger) {
            startVisualCapture();
        }
    };
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);

#ifdef Q_OS_WIN
    // Ensure proper removal of tray icon when program quits on Windows.
    connect(
      qApp, &QCoreApplication::aboutToQuit, m_trayIcon, &QSystemTrayIcon::hide);
#endif

    m_trayIcon->show();
}

void Controller::disableTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_trayIcon) {
        m_trayIcon->deleteLater();
    }
    ConfigHandler().setDisabledTrayIcon(true);
#endif
}

void Controller::sendTrayNotification(const QString& text,
                                      const QString& title,
                                      const int timeout)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(
          title, text, QIcon(":img/app/flameshot.svg"), timeout);
    }
}

void Controller::updateConfigComponents()
{
    if (m_configWindow) {
        m_configWindow->updateChildren();
    }
}

void Controller::updateRecentScreenshots()
{
    if (nullptr != m_history) {
        if (m_history->isVisible()) {
            m_history->loadHistory();
        }
    }
}

void Controller::showRecentScreenshots()
{
    if (nullptr == m_history) {
        m_history = new HistoryWidget();
    }
    m_history->loadHistory();
    m_history->show();
}

void Controller::sendCaptureSaved(uint id, const QString& savePath)
{
    emit captureSaved(id, savePath);
}

void Controller::startFullscreenCapture(const uint id)
{
    bool ok = true;
    QPixmap p(ScreenGrabber().grabEntireDesktop(ok));
    if (ok) {
        QRect selection; // `flameshot full` does not support --selection
        emit captureTaken(id, p, selection);
    } else {
        emit captureFailed(id);
    }
}

void Controller::handleCaptureTaken(uint id, QPixmap p, QRect selection)
{
    auto it = m_requestMap.find(id);
    if (it != m_requestMap.end()) {
        it.value().exportCapture(p);
        m_requestMap.erase(it);
    }
}

void Controller::handleCaptureFailed(uint id)
{
    m_requestMap.remove(id);
}

void Controller::doLater(int msec, QObject* receiver, lambda func)
{
    QTimer* timer = new QTimer(receiver);
    QObject::connect(timer, &QTimer::timeout, receiver, [timer, func]() {
        func();
        timer->deleteLater();
    });
    timer->setInterval(msec);
    timer->start();
}
