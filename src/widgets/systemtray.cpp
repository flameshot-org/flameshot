#include "src/widgets/systemtray.h"

#include "src/core/controller.h"
#include "src/utils/globalvalues.h"

#include "src/utils/confighandler.h"
#include <QApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QUrl>

SystemTray::SystemTray(QObject* parent)
  : QSystemTrayIcon(parent)
{
    initMenu();

    setToolTip(QStringLiteral("Flameshot"));
#if defined(Q_OS_MACOS)
    // Because of the following issues on MacOS "Catalina":
    // https://bugreports.qt.io/browse/QTBUG-86393
    // https://developer.apple.com/forums/thread/126072
    auto currentMacOsVersion = QOperatingSystemVersion::current();
    if (currentMacOsVersion >= currentMacOsVersion.MacOSBigSur) {
        setContextMenu(m_menu);
    }
#else
    setContextMenu(m_menu);
#endif
    QIcon icon =
      QIcon::fromTheme("flameshot-tray", QIcon(GlobalValues::iconPathPNG()));
    setIcon(icon);

#if defined(Q_OS_MACOS)
    if (currentMacOsVersion < currentMacOsVersion.MacOSBigSur) {
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
        connect(
          m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);
    }
#else
    connect(this, &SystemTray::activated, this, [this](ActivationReason r) {
        if (r == Trigger) {
            Controller::instance()->gui();
        }
    });
#endif

#ifdef Q_OS_WIN
    // Ensure proper removal of tray icon when program quits on Windows.
    connect(qApp, &QCoreApplication::aboutToQuit, this, &SystemTray::hide);
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
            [this]() {});
}

SystemTray::~SystemTray()
{
    delete m_menu;
}

QAction* SystemTray::appUpdates()
{
    return m_appUpdates;
}

void SystemTray::initMenu()
{
    m_menu = new QMenu();

    QAction* captureAction = new QAction(tr("&Take Screenshot"), this);
    connect(captureAction, &QAction::triggered, this, [this]() {
#if defined(Q_OS_MACOS)
        auto currentMacOsVersion = QOperatingSystemVersion::current();
        if (currentMacOsVersion >= currentMacOsVersion.MacOSBigSur) {
            gui();
        } else {
            // It seems it is not relevant for MacOS BigSur (Wait 400 ms to hide
            // the QMenu)
            doLater(400, this, [this]() { gui(); });
        }
#else
    // Wait 400 ms to hide the QMenu
    // FIXME doLater(400, this, [this]() { Controller::instance()->gui(); });
#endif
    });
    QAction* launcherAction = new QAction(tr("&Open Launcher"), this);
    connect(launcherAction,
            &QAction::triggered,
            Controller::instance(),
            &Controller::launcher);
    QAction* configAction = new QAction(tr("&Configuration"), this);
    connect(configAction,
            &QAction::triggered,
            Controller::instance(),
            &Controller::config);
    QAction* infoAction = new QAction(tr("&About"), this);
    connect(infoAction,
            &QAction::triggered,
            Controller::instance(),
            &Controller::info);

    m_appUpdates = new QAction(tr("Check for updates"), this);
    connect(
      m_appUpdates, &QAction::triggered, this, &SystemTray::checkForUpdates);

    QAction* quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    // recent screenshots
    QAction* recentAction = new QAction(tr("&Latest Uploads"), this);
    connect(recentAction, SIGNAL(triggered()), this, SLOT(history()));

    m_menu->addAction(captureAction);
    m_menu->addAction(launcherAction);
    m_menu->addSeparator();
    m_menu->addAction(recentAction);
    m_menu->addSeparator();
    m_menu->addAction(configAction);
    m_menu->addSeparator();
    m_menu->addAction(m_appUpdates);
    m_menu->addAction(infoAction);
    m_menu->addSeparator();
    m_menu->addAction(quitAction);
}

void SystemTray::enableCheckUpdatesAction(bool enable)
{
    if (m_appUpdates != nullptr) {
        m_appUpdates->setVisible(enable);
        m_appUpdates->setEnabled(enable);
    }
    if (enable) {
        Controller::instance()->getLatestAvailableVersion();
    }
}

void SystemTray::checkForUpdates()
{
    if (Controller::instance()->m_appLatestUrl.isEmpty()) {
        // FIXME m_showCheckAppUpdateStatus = true;
        Controller::instance()->getLatestAvailableVersion();
    } else {
        QDesktopServices::openUrl(QUrl(Controller::instance()->m_appLatestUrl));
    }
}
