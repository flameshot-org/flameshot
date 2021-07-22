// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "controller.h"

#if defined(Q_OS_MACOS)
#include "external/QHotkey/QHotkey"
#endif

#include "src/config/configwindow.h"
#include "src/core/qguiappcurrentscreen.h"
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
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOperatingSystemVersion>
#include <QSystemTrayIcon>
#include <QThread>

#ifdef Q_OS_WIN
#include "src/core/globalshortcutfilter.h"
#endif

#if defined(Q_OS_MACOS)
#include <QOperatingSystemVersion>
#include <QScreen>
#endif

// Controller is the core component of Flameshot, creates the trayIcon and
// launches the capture widget

Controller::Controller()
  : m_captureWindow(nullptr)
  , m_history(nullptr)
  , m_trayIcon(nullptr)
  , m_trayIconMenu(nullptr)
  , m_networkCheckUpdates(nullptr)
  , m_showCheckAppUpdateStatus(false)
#if defined(Q_OS_MACOS)
  , m_HotkeyScreenshotCapture(nullptr)
  , m_HotkeyScreenshotHistory(nullptr)
#endif
{
    m_appLatestVersion = QStringLiteral(APP_VERSION).replace("v", "");
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

#if defined(Q_OS_MACOS)
    // Try to take a test screenshot, MacOS will request a "Screen Recording"
    // permissions on the first run. Otherwise it will be hidden under the
    // CaptureWidget
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    currentScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1, 1);

    // set global shortcuts for MacOS
    m_HotkeyScreenshotCapture =
      new QHotkey(QKeySequence("Ctrl+Alt+Shift+4"), true, this);
    QObject::connect(m_HotkeyScreenshotCapture,
                     &QHotkey::activated,
                     qApp,
                     [&]() { this->startVisualCapture(); });
    m_HotkeyScreenshotHistory =
      new QHotkey(QKeySequence("Ctrl+Alt+Shift+H"), true, this);
    QObject::connect(m_HotkeyScreenshotHistory,
                     &QHotkey::activated,
                     qApp,
                     [&]() { this->showRecentScreenshots(); });
#endif

    if (ConfigHandler().checkForUpdates()) {
        getLatestAvailableVersion();
    }
}

Controller::~Controller()
{
    delete m_history;
    delete m_trayIconMenu;
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

void Controller::setCheckForUpdatesEnabled(const bool enabled)
{
    m_appUpdates->setVisible(enabled);
    m_appUpdates->setEnabled(enabled);
    if (enabled) {
        getLatestAvailableVersion();
    }
}

void Controller::getLatestAvailableVersion()
{
    // This features is required for MacOS and Windows user and for Linux users
    // who installed Flameshot not from the repository.
    m_networkCheckUpdates = new QNetworkAccessManager(this);
    QNetworkRequest requestCheckUpdates(QUrl(FLAMESHOT_APP_VERSION_URL));
    connect(m_networkCheckUpdates,
            &QNetworkAccessManager::finished,
            this,
            &Controller::handleReplyCheckUpdates);
    m_networkCheckUpdates->get(requestCheckUpdates);

    // check for updates each 24 hours
    doLater(1000 * 60 * 60 * 24, this, [this]() {
        if (ConfigHandler().checkForUpdates()) {
            this->getLatestAvailableVersion();
        }
    });
}

void Controller::handleReplyCheckUpdates(QNetworkReply* reply)
{
    if (!ConfigHandler().checkForUpdates()) {
        return;
    }
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json = response.object();
        m_appLatestVersion = json["tag_name"].toString().replace("v", "");

        // Transform strings version for correct comparison
        QStringList appLatestVersion =
          m_appLatestVersion.replace("v", "").split(".");
        QStringList currentVersion =
          QStringLiteral(APP_VERSION).replace("v", "").split(".");
        // transform versions to the string which can be compared correctly,
        // example: versions "0.8.5.9" and "0.8.5.10" are transformed into:
        // "0000.0008.0005.0009" and "0000.0008.0005.0010"
        // For string comparison you'll get:
        // "0.8.5.9" < "0.8.5.10" INCORRECT (lower version is bigger)
        // "0000.0008.0005.0009" > "0000.0008.0005.0010" CORRECT
        std::transform(
          appLatestVersion.begin(),
          appLatestVersion.end(),
          appLatestVersion.begin(),
          [](QString c) -> QString { return c = ("0000" + c).right(4); });
        std::transform(
          currentVersion.begin(),
          currentVersion.end(),
          currentVersion.begin(),
          [](QString c) -> QString { return c = ("0000" + c).right(4); });

        if (currentVersion.join(".").compare(appLatestVersion.join(".")) < 0) {
            m_appLatestUrl = json["html_url"].toString();
            QString newVersion =
              tr("New version %1 is available").arg(m_appLatestVersion);
            if (m_appUpdates != nullptr) {
                m_appUpdates->setText(newVersion);
            }
            if (m_showCheckAppUpdateStatus) {
                sendTrayNotification(newVersion, "Flameshot");
                QDesktopServices::openUrl(QUrl(m_appLatestUrl));
            }
        } else if (m_showCheckAppUpdateStatus) {
            sendTrayNotification(tr("You have the latest version"),
                                 "Flameshot");
        }
    } else {
        qWarning() << "Failed to get information about the latest version. "
                   << reply->errorString();
        if (m_showCheckAppUpdateStatus) {
            sendTrayNotification(
              tr("Failed to get information about the latest version."),
              "Flameshot");
        }
    }
    m_showCheckAppUpdateStatus = false;
}

void Controller::appUpdates()
{
    if (m_appLatestUrl.isEmpty()) {
        m_showCheckAppUpdateStatus = true;
        getLatestAvailableVersion();
    } else {
        QDesktopServices::openUrl(QUrl(m_appLatestUrl));
    }
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
            // TODO: Figure out the code path that gets here so the deprated
            // warning can be fixed
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
#if defined(Q_OS_MACOS)
    // This is required on MacOS because of Mission Control. If you'll switch to
    // another Desktop you cannot take a new screenshot from the tray, you have
    // to switch back to the Flameshot Desktop manually. It is not obvious and a
    // large number of users are confused and report a bug.
    if (m_captureWindow) {
        m_captureWindow->close();
        delete m_captureWindow;
        m_captureWindow = nullptr;
    }
#endif

    if (nullptr == m_captureWindow) {
        int timeout = 5000; // 5 seconds
        const int delay = 100;
        QWidget* modalWidget = nullptr;
        for (; timeout >= 0; timeout -= delay) {
            modalWidget = qApp->activeModalWidget();
            if (nullptr == modalWidget) {
                break;
            }
            modalWidget->close();
            modalWidget->deleteLater();
            QThread::msleep(delay);
        }
        if (0 == timeout) {
            QMessageBox::warning(
              nullptr, tr("Error"), tr("Unable to close active modal widgets"));
            return;
        }

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
#elif defined(Q_OS_MACOS)
        // In "Emulate fullscreen mode"
        m_captureWindow->showFullScreen();
        m_captureWindow->activateWindow();
        m_captureWindow->raise();
#else
        m_captureWindow->showFullScreen();
//        m_captureWindow->show(); // For CaptureWidget Debugging under Linux
#endif
        if (!m_appLatestUrl.isEmpty() &&
            ConfigHandler().ignoreUpdateToVersion().compare(
              m_appLatestVersion) < 0) {
            m_captureWindow->showAppUpdateNotification(m_appLatestVersion,
                                                       m_appLatestUrl);
        }
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
#if defined(Q_OS_MACOS)
        m_configWindow->activateWindow();
        m_configWindow->raise();
#endif
    }
}

// creation of the window of information
void Controller::openInfoWindow()
{
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
#if defined(Q_OS_MACOS)
        m_infoWindow->activateWindow();
        m_infoWindow->raise();
#endif
    }
}

void Controller::openLauncherWindow()
{
    if (!m_launcherWindow) {
        m_launcherWindow = new CaptureLauncher();
    }
    m_launcherWindow->show();
#if defined(Q_OS_MACOS)
    m_launcherWindow->activateWindow();
    m_launcherWindow->raise();
#endif
}

void Controller::enableTrayIcon()
{
    ConfigHandler().setDisabledTrayIcon(false);
    if (m_trayIcon) {
        m_trayIcon->show();
        return;
    }
    if (nullptr == m_trayIconMenu) {
        m_trayIconMenu = new QMenu();
        Q_ASSERT(m_trayIconMenu);
    }

    QAction* captureAction = new QAction(tr("&Take Screenshot"), this);
    connect(captureAction, &QAction::triggered, this, [this]() {
#if defined(Q_OS_MACOS)
        auto currentMacOsVersion = QOperatingSystemVersion::current();
        if (currentMacOsVersion >= currentMacOsVersion.MacOSBigSur) {
            startVisualCapture();
        } else {
            // It seems it is not relevant for MacOS BigSur (Wait 400 ms to hide
            // the QMenu)
            doLater(400, this, [this]() { this->startVisualCapture(); });
        }
#else
      // Wait 400 ms to hide the QMenu
        doLater(400, this, [this]() { this->startVisualCapture(); });
#endif
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

    m_appUpdates = new QAction(tr("Check for updates"), this);
    connect(m_appUpdates, &QAction::triggered, this, &Controller::appUpdates);

    QAction* quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    // recent screenshots
    QAction* recentAction = new QAction(tr("&Latest Uploads"), this);
    connect(
      recentAction, SIGNAL(triggered()), this, SLOT(showRecentScreenshots()));

    // generate menu
    m_trayIconMenu->addAction(captureAction);
    m_trayIconMenu->addAction(launcherAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(recentAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(configAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_appUpdates);
    m_trayIconMenu->addAction(infoAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(quitAction);
    setCheckForUpdatesEnabled(ConfigHandler().checkForUpdates());

    if (nullptr == m_trayIcon) {
        m_trayIcon = new QSystemTrayIcon();
        Q_ASSERT(m_trayIcon);
    }
    m_trayIcon->setToolTip(QStringLiteral("Flameshot"));
#if defined(Q_OS_MACOS)
    // Because of the following issues on MacOS "Catalina":
    // https://bugreports.qt.io/browse/QTBUG-86393
    // https://developer.apple.com/forums/thread/126072
    auto currentMacOsVersion = QOperatingSystemVersion::current();
    if (currentMacOsVersion >= currentMacOsVersion.MacOSBigSur) {
        m_trayIcon->setContextMenu(m_trayIconMenu);
    }
#else
    m_trayIcon->setContextMenu(m_trayIconMenu);
#endif
    QIcon trayIcon =
      QIcon::fromTheme("flameshot-tray", QIcon(":img/app/flameshot.png"));
    m_trayIcon->setIcon(trayIcon);

#if defined(Q_OS_MACOS)
    if (currentMacOsVersion < currentMacOsVersion.MacOSBigSur) {
        // Because of the following issues on MacOS "Catalina":
        // https://bugreports.qt.io/browse/QTBUG-86393
        // https://developer.apple.com/forums/thread/126072
        auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r) {
            if (m_trayIconMenu->isVisible()) {
                m_trayIconMenu->hide();
            } else {
                m_trayIconMenu->popup(QCursor::pos());
            }
        };
        connect(
          m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);
    }
#else
    auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::Trigger) {
            startVisualCapture();
        }
    };
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);
#endif

#ifdef Q_OS_WIN
    // Ensure proper removal of tray icon when program quits on Windows.
    connect(
      qApp, &QCoreApplication::aboutToQuit, m_trayIcon, &QSystemTrayIcon::hide);
#endif

    m_trayIcon->show();

    if (ConfigHandler().showStartupLaunchMessage()) {
        m_trayIcon->showMessage(
          "Flameshot",
          QObject::tr(
            "Hello, I'm here! Click icon in the tray to take a screenshot or "
            "click with a right button to see more options."),
          trayIcon,
          3000);
    }
}

void Controller::disableTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) || defined(Q_OS_MACOS)
    if (m_trayIcon) {
        m_trayIcon->hide();
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
#if defined(Q_OS_MACOS)
    m_history->activateWindow();
    m_history->raise();
#endif
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
