#include "trayicon.h"

#include "src/core/flameshot.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/globalvalues.h"

#include "src/utils/confighandler.h"
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
            [this]() { updateCaptureActionShortcut(); });
}

TrayIcon::~TrayIcon()
{
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
    m_infoAction = new QAction(tr("&About"), this);
    connect(m_infoAction,
            &QAction::triggered,
            Flameshot::instance(),
            &Flameshot::info);

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
                if (ConfigHandler().checkForUpdates()) {
                    QString newVersion =
                      tr("Download version %1").arg(version.toString());
                    m_appUpdates->setText(newVersion);
                    m_appUpdates->setVisible(true);

                    // hack to work around menu not updating when the text /
                    // visibility is modified Force menu refresh by removing and
                    // re-adding the action
                    m_menu->removeAction(m_appUpdates);
                    m_menu->insertAction(m_infoAction, m_appUpdates);
                }
            });
    updateCheckUpdatesMenuVisibility();
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
    m_menu->addAction(configAction);
    m_menu->addSeparator();
#if !defined(DISABLE_UPDATE_CHECKER)
    m_menu->addAction(m_appUpdates);
#endif
    m_menu->addAction(m_infoAction);
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

#if !defined(DISABLE_UPDATE_CHECKER)
void TrayIcon::updateCheckUpdatesMenuVisibility()
{
    if (m_appUpdates == nullptr) {
        return;
    }

    bool autoCheckEnabled = ConfigHandler().checkForUpdates();
    if (autoCheckEnabled) {
        // When auto-check is enabled, hide the menu item initially
        // It will be shown when a new version is available via a callback
        m_appUpdates->setVisible(false);
    } else {
        m_appUpdates->setVisible(true);
        m_appUpdates->setText(tr("Check for updates"));
    }
}
#endif

void TrayIcon::startGuiCapture()
{
    auto* widget = Flameshot::instance()->gui();
#if !defined(DISABLE_UPDATE_CHECKER)
    FlameshotDaemon::instance()->showUpdateNotificationIfAvailable(widget);
#endif
}
