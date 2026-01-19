#include "trayicon.h"

#include "countdownwindow.h"
#include "src/core/flameshot.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/globalvalues.h"

#include "src/utils/confighandler.h"
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>

#if defined(Q_OS_MACOS)
#include <QOperatingSystemVersion>
#endif

TrayIcon::TrayIcon(QObject* parent)
  : QSystemTrayIcon(parent)
  , m_delayMenu(nullptr)
  , m_delayActionGroup(nullptr)
  , m_countdownTimer(nullptr)
  , m_remainingSeconds(0)
{
    initMenu();

    setToolTip(QStringLiteral("Flameshot"));
#if defined(Q_OS_MACOS)
    // Because of the following issues on MacOS "Catalina":
    // https://bugreports.qt.io/browse/QTBUG-86393
    // https://developer.apple.com/forums/thread/126072
    auto currentMacOsVersion = QOperatingSystemVersion::current();
    if (currentMacOsVersion >= QOperatingSystemVersion::MacOSBigSur) {
        setContextMenu(m_menu);
    }
#else
    setContextMenu(m_menu);
#endif
    QIcon icon =
      QIcon::fromTheme("flameshot-tray", QIcon(GlobalValues::trayIconPath()));

#if defined(Q_OS_MACOS)
    if (currentMacOsVersion >= QOperatingSystemVersion::MacOSBigSur) {
        icon.setIsMask(true);
    }
#endif

    setIcon(icon);

#if defined(Q_OS_MACOS)
    if (currentMacOsVersion < QOperatingSystemVersion::MacOSBigSur) {
        // Because of the following issues on MacOS "Catalina":
        // https://bugreports.qt.io/browse/QTBUG-86393
        // https://developer.apple.com/forums/thread/126072
        auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r) {
            if (m_menu->isVisible()) {
                m_menu->hide();
            } else {
                m_menu->popup(QCursor::pos());
            }
        };
        connect(this, &QSystemTrayIcon::activated, this, trayIconActivated);
    }
#else
    connect(this, &TrayIcon::activated, this, [this](ActivationReason r) {
        if (r == Trigger) {
            startGuiCapture();
        }
    });
#endif

#ifdef Q_OS_WIN
    // Ensure proper removal of tray icon when program quits on Windows.
    connect(qApp, &QCoreApplication::aboutToQuit, this, &TrayIcon::hide);
#endif

    show(); // TODO needed?

    if (ConfigHandler().showStartupLaunchMessage()) {
        showMessage(
          "Flameshot",
          QObject::tr(
            "Hello, I'm here! Click icon in the tray to take a screenshot or "
            "click with a right button to see more options."),
          icon,
          3000);
    }

    connect(ConfigHandler::getInstance(),
            &ConfigHandler::fileChanged,
            this,
            [this]() {
                updateCaptureActionShortcut();
                updateDelayActions();
            });
}

TrayIcon::~TrayIcon()
{
    if (m_countdownTimer) {
        m_countdownTimer->stop();
        delete m_countdownTimer;
    }
    delete m_delayMenu;
    delete m_delayActionGroup;
    delete m_menu;
}

#if !defined(DISABLE_UPDATE_CHECKER)
QAction* TrayIcon::appUpdates()
{
    return m_appUpdates;
}
#endif

void TrayIcon::initMenu()
{
    m_menu = new QMenu();

    m_captureAction = new QAction(tr("&Take Screenshot"), this);

    updateCaptureActionShortcut();

    connect(m_captureAction, &QAction::triggered, this, [this]() {
#if defined(Q_OS_MACOS)
        auto currentMacOsVersion = QOperatingSystemVersion::current();
        if (currentMacOsVersion >= QOperatingSystemVersion::MacOSBigSur) {
            startGuiCapture();
        } else {
            // It seems it is not relevant for MacOS BigSur (Wait 400 ms to hide
            // the QMenu)
            QTimer::singleShot(400, this, [this]() { startGuiCapture(); });
        }
#else
    // Wait 400 ms to hide the QMenu
    QTimer::singleShot(400, this, [this]() {
        startGuiCapture();
    });
#endif
    });

    // Create delay submenu
    m_delayMenu = new QMenu(tr("Screenshot &Delay"), m_menu);
    m_delayActionGroup = new QActionGroup(this);
    m_delayActionGroup->setExclusive(true);

    // Define delay options (in milliseconds)
    QVector<QPair<QString, int>> delayOptions = { { tr("No Delay"), 0 },
                                                  { tr("1 second"), 1000 },
                                                  { tr("3 seconds"), 3000 },
                                                  { tr("5 seconds"), 5000 },
                                                  { tr("10 seconds"), 10000 } };

    // Get current delay from config (default to 0)
    int currentDelay = ConfigHandler().captureDelay();

    for (const auto& option : delayOptions) {
        QAction* delayAction = new QAction(option.first, m_delayMenu);
        delayAction->setCheckable(true);
        delayAction->setData(option.second);

        if (option.second == currentDelay) {
            delayAction->setChecked(true);
        }

        connect(delayAction, &QAction::triggered, this, [this, delayAction]() {
            int delay = delayAction->data().toInt();
            ConfigHandler().setCaptureDelay(delay);
        });

        m_delayActionGroup->addAction(delayAction);
        m_delayMenu->addAction(delayAction);
    }

    auto* launcherAction = new QAction(tr("&Open Launcher"), this);
    connect(launcherAction,
            &QAction::triggered,
            Flameshot::instance(),
            &Flameshot::launcher);
    auto* configAction = new QAction(tr("&Configuration"), this);
    connect(configAction,
            &QAction::triggered,
            Flameshot::instance(),
            &Flameshot::config);
    auto* infoAction = new QAction(tr("&About"), this);
    connect(
      infoAction, &QAction::triggered, Flameshot::instance(), &Flameshot::info);

#if !defined(DISABLE_UPDATE_CHECKER)
    m_appUpdates = new QAction(tr("Check for updates"), this);
    connect(m_appUpdates,
            &QAction::triggered,
            FlameshotDaemon::instance(),
            &FlameshotDaemon::checkForUpdates);

    connect(FlameshotDaemon::instance(),
            &FlameshotDaemon::newVersionAvailable,
            this,
            [this](const QVersionNumber& version) {
                QString newVersion =
                  tr("New version %1 is available").arg(version.toString());
                m_appUpdates->setText(newVersion);
            });
#endif

    QAction* quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

#ifdef ENABLE_IMGUR
    // recent screenshots
    QAction* recentAction = new QAction(tr("&Latest Uploads"), this);
    connect(recentAction,
            &QAction::triggered,
            Flameshot::instance(),
            &Flameshot::history);
#endif
    auto* openSavePathAction = new QAction(tr("&Open Save Path"), this);
    connect(openSavePathAction,
            &QAction::triggered,
            Flameshot::instance(),
            &Flameshot::openSavePath);

    m_menu->addAction(m_captureAction);
    m_menu->addAction(launcherAction);
    m_menu->addSeparator();
#ifdef ENABLE_IMGUR
    m_menu->addAction(recentAction);
#endif
    m_menu->addAction(openSavePathAction);
    m_menu->addSeparator();
    m_menu->addMenu(m_delayMenu);
    m_menu->addAction(configAction);
    m_menu->addSeparator();
#if !defined(DISABLE_UPDATE_CHECKER)
    m_menu->addAction(m_appUpdates);
#endif
    m_menu->addAction(infoAction);
    m_menu->addSeparator();
    m_menu->addAction(quitAction);
}

void TrayIcon::updateCaptureActionShortcut()
{
#if defined(Q_OS_MACOS)
    if (!m_captureAction) {
        return;
    }

    QString shortcut = ConfigHandler().shortcut("TAKE_SCREENSHOT");
    m_captureAction->setShortcut(QKeySequence(shortcut));
#endif
}

void TrayIcon::updateDelayActions()
{
    if (!m_delayActionGroup) {
        return;
    }

    int currentDelay = ConfigHandler().captureDelay();

    for (QAction* action : m_delayActionGroup->actions()) {
        if (action->data().toInt() == currentDelay) {
            action->setChecked(true);
            break;
        }
    }
}

#if !defined(DISABLE_UPDATE_CHECKER)
void TrayIcon::enableCheckUpdatesAction(bool enable)
{
    if (m_appUpdates != nullptr) {
        m_appUpdates->setVisible(enable);
        m_appUpdates->setEnabled(enable);
    }
    if (enable) {
        FlameshotDaemon::instance()->getLatestAvailableVersion();
    }
}
#endif

void TrayIcon::startGuiCapture()
{
    // Get the configured delay
    int delay = ConfigHandler().captureDelay();

    if (delay > 0) {
        startGuiCaptureWithCountdown(delay);
    } else {
        // No delay, start immediately
        auto* widget = Flameshot::instance()->gui();
#if !defined(DISABLE_UPDATE_CHECKER)
        FlameshotDaemon::instance()->showUpdateNotificationIfAvailable(widget);
#endif
    }
}

void TrayIcon::startGuiCaptureWithCountdown(int delayMs)
{
    int seconds = delayMs / 1000;

    // Create countdown window
    auto* countdownWindow = new CountdownWindow(seconds);

    // When countdown finishes, take the screenshot
    connect(
      countdownWindow, &CountdownWindow::countdownFinished, this, [this]() {
          // Add a small delay to ensure the window is fully closed
          QTimer::singleShot(50, this, [this]() {
              auto* widget = Flameshot::instance()->gui();
#if !defined(DISABLE_UPDATE_CHECKER)
              FlameshotDaemon::instance()->showUpdateNotificationIfAvailable(
                widget);
#endif
          });
      });

    countdownWindow->startCountdown();
}
