#include <QList>// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "controller.h"
#include "flameshotdaemon.h"

#if defined(Q_OS_MACOS)
#include "external/QHotkey/QHotkey"
#endif

#include "abstractlogger.h"
#include "pinwidget.h"
#include "screenshotsaver.h"
#include "src/config/configresolver.h"
#include "src/config/configwindow.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/tools/imgupload/imguploadermanager.h"
#include "src/tools/imgupload/storages/imguploaderbase.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include "src/utils/history.h"
#include "src/utils/screengrabber.h"
#include "src/widgets/capture/capturetoolbutton.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/widgets/capturelauncher.h"
#include "src/widgets/historywidget.h"
#include "src/widgets/imguploaddialog.h"
#include "src/widgets/infowindow.h"
#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
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
#include <QVersionNumber>

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

    QString StyleSheet = CaptureButton::globalStyleSheet();
    qApp->setStyleSheet(StyleSheet);

#if defined(Q_OS_MACOS)
    // Try to take a test screenshot, MacOS will request a "Screen Recording"
    // permissions on the first run. Otherwise it will be hidden under the
    // CaptureWidget
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    currentScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1, 1);

    // set global shortcuts for MacOS
    m_HotkeyScreenshotCapture = new QHotkey(
      QKeySequence(ConfigHandler().shortcut("TAKE_SCREENSHOT")), true, this);
    QObject::connect(m_HotkeyScreenshotCapture,
                     &QHotkey::activated,
                     qApp,
                     [&]() { this->startVisualCapture(); });
    m_HotkeyScreenshotHistory = new QHotkey(
      QKeySequence(ConfigHandler().shortcut("SCREENSHOT_HISTORY")), true, this);
    QObject::connect(m_HotkeyScreenshotHistory,
                     &QHotkey::activated,
                     qApp,
                     [&]() { this->showRecentUploads(); });
#endif
    connect(ConfigHandler::getInstance(),
            &ConfigHandler::fileChanged,
            this,
            [this]() {
                ConfigHandler config;
                if (config.disabledTrayIcon()) {
                    disableTrayIcon();
                } else {
                    enableTrayIcon();
                }
            });

    if (ConfigHandler().checkForUpdates()) {
        getLatestAvailableVersion();
    }
}

Controller::~Controller()
{
    delete m_trayIconMenu;
}

Controller* Controller::getInstance()
{
    static Controller c;
    return &c;
}

void Controller::setCheckForUpdatesEnabled(const bool enabled)
{
    if (m_appUpdates != nullptr) {
        m_appUpdates->setVisible(enabled);
        m_appUpdates->setEnabled(enabled);
    }
    if (enabled) {
        getLatestAvailableVersion();
    }
}

void Controller::setOrigin(Origin origin)
{
    m_origin = origin;
}

Controller::Origin Controller::origin()
{
    return m_origin;
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

/**
 * @brief Prompt the user to resolve config errors if necessary.
 * @return Whether errors were resolved.
 */
bool Controller::resolveAnyConfigErrors()
{
    bool resolved = true;
    ConfigHandler config;
    if (!config.checkUnrecognizedSettings() || !config.checkSemantics()) {
        ConfigResolver* resolver = new ConfigResolver();
        QObject::connect(
          resolver, &ConfigResolver::rejected, [this, resolver, &resolved]() {
              resolved = false;
              resolver->deleteLater();
              if (origin() == CLI) {
                  exit(1);
              }
          });
        QObject::connect(
          resolver, &ConfigResolver::accepted, [resolver, &resolved]() {
              resolved = true;
              resolver->close();
              resolver->deleteLater();
              // Ensure that the dialog is closed before starting capture
              qApp->processEvents();
          });
        resolver->exec();
        qApp->processEvents();
    }
    return resolved;
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

        QVersionNumber appLatestVersion =
          QVersionNumber::fromString(m_appLatestVersion);
        QVersionNumber currentVersion = QVersionNumber::fromString(
          QStringLiteral(APP_VERSION).replace("v", ""));

        if (currentVersion < appLatestVersion) {
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
    if (!resolveAnyConfigErrors())
        return;

    switch (request.captureMode()) {
        case CaptureRequest::FULLSCREEN_MODE:
            doLater(request.delay(), this, [this, request]() {
                startFullscreenCapture(request);
            });
            break;
        case CaptureRequest::SCREEN_MODE: {
            int&& number = request.data().toInt();
            doLater(request.delay(), this, [this, request, number]() {
                startScreenGrab(request, number);
            });
            break;
        }
        case CaptureRequest::GRAPHICAL_MODE: {
            doLater(request.delay(), this, [this, request]() {
                startVisualCapture(request);
            });
            break;
        }
        default:
            handleCaptureFailed();
            break;
    }
}

// creation of a new capture in GUI mode
void Controller::startVisualCapture(const CaptureRequest& req)
{
    if (!resolveAnyConfigErrors())
        return;

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
        // TODO is this unnecessary now?
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

        m_captureWindow = new CaptureWidget(req);
        // m_captureWindow = new CaptureWidget(forcedSavePath, false); //
        // debug

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
        emit captureFailed();
    }
}

void Controller::startScreenGrab(CaptureRequest req, const int screenNumber)
{
    if (!resolveAnyConfigErrors())
        return;

    bool ok = true;
    QScreen* screen;

    if (screenNumber < 0) {
        QPoint globalCursorPos = QCursor::pos();
#if QT_VERSION > QT_VERSION_CHECK(5, 10, 0)
        screen = qApp->screenAt(globalCursorPos);
#else
        screen =
          qApp->screens()[qApp->desktop()->screenNumber(globalCursorPos)];
#endif
    } else {
        screen = qApp->screens()[screenNumber];
    }
    QPixmap p(ScreenGrabber().grabScreen(screen, ok));
    if (ok) {
        QRect geometry = ScreenGrabber().screenGeometry(screen);
        QRect region = req.initialSelection();
        if (region.isNull()) {
            region = ScreenGrabber().screenGeometry(screen);
        } else {
            QRect screenGeom = ScreenGrabber().screenGeometry(screen);
            screenGeom.moveTopLeft({ 0, 0 });
            region = region.intersected(screenGeom);
            p = p.copy(region);
        }
        if (req.tasks() & CaptureRequest::PIN) {
            // change geometry for pin task
            req.addPinTask(region);
        }
        exportCapture(p, geometry, req);
    } else {
        handleCaptureFailed();
    }
}

// creation of the configuration window
void Controller::openConfigWindow()
{
    if (!resolveAnyConfigErrors())
        return;

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
    if (!resolveAnyConfigErrors())
        return;

    if (!m_launcherWindow) {
        m_launcherWindow = new CaptureLauncher();
    }
    m_launcherWindow->show();
#if defined(Q_OS_MACOS)
    m_launcherWindow->activateWindow();
    m_launcherWindow->raise();
#endif
}

void Controller::initTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (!ConfigHandler().disabledTrayIcon()) {
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
        doLater(400, this, [this]() { startVisualCapture(); });
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
    connect(recentAction, SIGNAL(triggered()), this, SLOT(showRecentUploads()));

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
      QIcon::fromTheme("flameshot-tray", QIcon(GlobalValues::iconPathPNG()));
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
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
    ConfigHandler().setDisabledTrayIcon(true);
}

void Controller::sendTrayNotification(const QString& text,
                                      const QString& title,
                                      const int timeout)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(
          title, text, QIcon(GlobalValues::iconPath()), timeout);
    }
}

void Controller::showRecentUploads()
{
    static HistoryWidget* historyWidget = nullptr;
    if (nullptr == historyWidget) {
        historyWidget = new HistoryWidget();
        connect(historyWidget, &QObject::destroyed, this, []() {
            historyWidget = nullptr;
        });
    }
    historyWidget->loadHistory();
    historyWidget->show();
#if defined(Q_OS_MACOS)
    historyWidget->activateWindow();
    historyWidget->raise();
#endif
}

void Controller::exportCapture(QPixmap capture,
                               QRect& selection,
                               const CaptureRequest& req)
{
    using CR = CaptureRequest;
    int tasks = req.tasks(), mode = req.captureMode();
    QString path = req.path();

    if (tasks & CR::PRINT_GEOMETRY) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        QTextStream(stdout)
          << selection.width() << "x" << selection.height() << "+"
          << selection.x() << "+" << selection.y() << "\n";
    }

    if (tasks & CR::PRINT_RAW) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        capture.save(&buffer, "PNG");
        QFile file;
        file.open(stdout, QIODevice::WriteOnly);

        file.write(byteArray);
        file.close();
    }

    if (tasks & CR::SAVE) {
        if (req.path().isEmpty()) {
            ScreenshotSaver().saveToFilesystemGUI(capture);
        } else {
            ScreenshotSaver().saveToFilesystem(capture, path);
        }
    }

    if (tasks & CR::COPY) {
        FlameshotDaemon::copyToClipboard(capture);
    }

    if (tasks & CR::PIN) {
        FlameshotDaemon::createPin(capture, selection);
        if (mode == CR::SCREEN_MODE || mode == CR::FULLSCREEN_MODE) {
            AbstractLogger::info()
              << QObject::tr("Full screen screenshot pinned to screen");
        }
    }

    if (tasks & CR::UPLOAD) {
        if (!ConfigHandler().uploadWithoutConfirmation()) {
            ImgUploadDialog* dialog = new ImgUploadDialog();
            if (dialog->exec() == QDialog::Rejected) {
                return;
            }
        }

        ImgUploaderBase* widget = ImgUploaderManager().uploader(capture);
        widget->show();
        widget->activateWindow();
        // NOTE: lambda can't capture 'this' because it might be destroyed later
        CR::ExportTask tasks = tasks;
        QObject::connect(
          widget, &ImgUploaderBase::uploadOk, [=](const QUrl& url) {
              if (ConfigHandler().copyAndCloseAfterUpload()) {
                  if (!(tasks & CR::COPY)) {
                      FlameshotDaemon::copyToClipboard(
                        url.toString(), tr("URL copied to clipboard."));
                      widget->close();
                  } else {
                      widget->showPostUploadDialog();
                  }
              } else {
                  widget->showPostUploadDialog();
              }
          });
    }

    if (!(tasks & CR::UPLOAD)) {
        emit captureTaken(capture, selection);
    }
}

void Controller::startFullscreenCapture(const CaptureRequest& req)
{
    if (!resolveAnyConfigErrors())
        return;

    bool ok = true;
    QPixmap p(ScreenGrabber().grabEntireDesktop(ok));
    QRect region = req.initialSelection();
    if (!region.isNull()) {
        p = p.copy(region);
    }
    if (ok) {
        QRect selection; // `flameshot full` does not support --selection
        exportCapture(p, selection, req);
    } else {
        handleCaptureFailed();
    }
}

void Controller::handleCaptureTaken(const CaptureRequest& req,
                                    QPixmap p,
                                    QRect selection)
{
    exportCapture(p, selection, req);
}

void Controller::handleCaptureFailed()
{
    emit captureFailed();
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

// STATIC ATTRIBUTES
Controller::Origin Controller::m_origin = Controller::DAEMON;
