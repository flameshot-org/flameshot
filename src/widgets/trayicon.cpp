#include "trayicon.h"
#include "core/capturerequest.h"
#include "core/flameshot.h"
#include "core/flameshotdaemon.h"
#include "core/qguiappcurrentscreen.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "qcollator.h"
#include "qdir.h"
#include "qmessagebox.h"
#include "qprocess.h"
#include "qregularexpression.h"
#include "qstandardpaths.h"
#include "screenshotsaver.h"
#include <QApplication>
#include <QGuiApplication>
#include <QMenu>
#include <QScreen>
#include <QTimer>
#include <QUrl>
#include <QVersionNumber>
#include <algorithm>
#include <iostream>
#include <QThread>

#if defined(Q_OS_MACOS)
#include <QOperatingSystemVersion>
#endif

#include <capturescreenscroll.h>
#if defined( Q_OS_LINUX )

//#include "tools/overlay/overlaytool.h"
#include <X11/extensions/XTest.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#endif

TrayIcon::TrayIcon(QObject* parent)
  : QSystemTrayIcon(parent)
  , m_screenMenu(nullptr)
{
    initMenu();
    initScreenMenu();

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

    auto* captureActionWithDesplazamiento = new QAction(tr("&Scrolling screenshot"), this);
    connect( captureActionWithDesplazamiento, &QAction::triggered, this, [ this ]() {

        QMessageBox msgCapturaWithScroll;

        msgCapturaWithScroll.setText( QObject::tr("Select the sale you want to capture with a scroll and leave the mouse over it until you finish and the \"Save Image\" text box appears..." ) );
        msgCapturaWithScroll.exec();

#if defined( Q_OS_LINUX )

        Display* display = XOpenDisplay( nullptr );
        if ( !display ) {
            qWarning() << "No se pudo abrir X11 Display.";
            return;
        }

        Window root = DefaultRootWindow( display );

        qDebug() << "Please click on the window you want to capture (waiting for xwininfo)...";
        WId targetWindowId = getWindowIdFromXwininfo();

        if ( targetWindowId == 0 ) {
            qDebug() << "Failed to get window ID.";
            return;
        }


        XWindowAttributes winAttr;
        if ( !XGetWindowAttributes( display, targetWindowId, &winAttr ) ) {
            qDebug() << "Error: ID de ventana no válido" << targetWindowId;
            return;
        }

        int xOffset = 0;
        int yOffset = 50;
        int width = winAttr.width;
        int height = winAttr.height;

        QPixmap previousCapture;
        int i = 0;

        QString picturesPath = QStandardPaths::writableLocation( QStandardPaths::PicturesLocation );
        QString baseDir = picturesPath + "/FlameshotCapture";
        QDir().mkpath( baseDir );

        captureScreenScroll* captureSS = new captureScreenScroll( static_cast<void*>( display ), targetWindowId, xOffset, yOffset, width, height );

        while ( true ) {

            QPixmap currentCapture = captureSS -> captureScrollableArea();

            if ( currentCapture.isNull() ) {
                qDebug() << "❌ Captura nula. Terminando.";
                break;
            }

            if ( captureSS -> imagesEqual( previousCapture.toImage(), currentCapture.toImage() ) ) {
                qDebug() << "⚠️ Imagen no cambió. Deteniendo scroll.";
                break;
            }

            QString const filename = QString( "captura%1.png" ).arg( i );
            currentCapture.save( baseDir + "/" + filename );
            qDebug() << "📸 Captura guardada:" << filename;
            previousCapture = currentCapture;

            // Scroll hacia abajo
            XTestFakeButtonEvent( display, 5, True, CurrentTime );
            XFlush( display );
            XTestFakeButtonEvent( display, 5, False, CurrentTime );
            XFlush( display );
            usleep( 200000 );

            i++;
        }
#endif
        // Post-procesamiento
        struct Item { std::string path; std::string name; };
        std::vector<Item> items;

        for (const auto& e : fs::directory_iterator( baseDir.toStdString() ) )
            if ( e.is_regular_file() && e.path().extension() == ".png" )
                items.push_back( { e.path().string(), e.path().filename().string() } );

        QCollator coll;
        coll.setNumericMode( true );
        coll.setCaseSensitivity( Qt::CaseInsensitive );

        std::stable_sort( items.begin(), items.end(),
                         [ & ]( const Item& a, const Item& b ) {
                             return coll.compare( QString::fromUtf8( a.name ),
                                                  QString::fromUtf8( b.name ) ) < 0;
                         } );

        std::vector<std::string> paths;
        for ( const auto& i : items )
            paths.push_back( i.path );

        cv::Mat result;
        cv::Mat firstRefImage;

        for ( const auto& f : paths ) {
            qDebug() << "procesando " << QString::fromStdString( f ) << '\n';
            cv::Mat img = cv::imread( f );
            if ( img.empty() ) {
                std::cerr << "No se lee " << f << '\n';
                continue;
            }

            cv::Mat cropped = captureSS -> cropHorizontal( img );

            if ( result.empty() ) {
                result = cropped.clone();
                firstRefImage = cropped.clone();
                continue;
            }

            result = captureSS -> combineImages( result, cropped );
        }

        QPixmap finalImage = captureSS -> cvMatToQPixmap( result );

        if ( !finalImage.isNull() ) {
            cv::imwrite( baseDir.toStdString() + "/resultado_stitched.png", result );
            saveToFilesystemGUI( finalImage );
            QString dirPath = baseDir;
            QDir( dirPath ).removeRecursively();
        } else {
            qDebug() << "Failed to stitch images.";
        }

    } );

    m_launcherAction = new QAction(tr("&Open Launcher"), this);
    connect(m_launcherAction,
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
    m_menu->addAction(captureActionWithDesplazamiento);
    m_menu->addAction(m_launcherAction);
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

void TrayIcon::initScreenMenu()
{
#ifndef Q_OS_MACOS
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.size() <= 1) {
        return;
    }

    m_screenMenu = new QMenu(tr("Select Screen"));

    QList<QAction*> actions = m_menu->actions();
    int index = actions.indexOf(m_launcherAction);
    if (index >= 0 && index + 1 < actions.size()) {
        m_menu->insertMenu(actions[index + 1], m_screenMenu);
    } else {
        m_menu->addMenu(m_screenMenu);
    }

    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    int currentIndex = screens.indexOf(currentScreen);

    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        QRect geom = screen->geometry();
        QString screenDescription = tr("Monitor %1: %2 (%3x%4)")
                                      .arg(i + 1)
                                      .arg(screen->name())
                                      .arg(geom.width())
                                      .arg(geom.height());

        QAction* screenAction = m_screenMenu->addAction(screenDescription);
        connect(screenAction, &QAction::triggered, this, [this, i]() {
            QTimer::singleShot(
              100, this, [this, i]() { startGuiCaptureOnScreen(i); });
        });
    }
#endif
}

void TrayIcon::startGuiCapture()
{
    auto* widget = Flameshot::instance()->gui();
#if !defined(DISABLE_UPDATE_CHECKER)
    FlameshotDaemon::instance()->showUpdateNotificationIfAvailable(widget);
#endif
}

void TrayIcon::startGuiCaptureOnScreen(int screenIndex)
{
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, 400);
    req.setSelectedMonitor(screenIndex);
    Flameshot::instance()->requestCapture(req);
}

#if defined( Q_OS_LINUX )
WId TrayIcon::getWindowIdFromXwininfo()
{
    QProcess process;
    process.start( "xwininfo" );
    process.waitForFinished( -1 );

    QString output = process.readAllStandardOutput();
    qDebug() << "xwininfo output:\n" << output;

    QRegularExpression re( "Window id: (0x[0-9a-fA-F]+)" );
    QRegularExpressionMatch match = re.match( output );

    if ( match.hasMatch() ) {
        QString windowIdStr = match.captured( 1 );
        bool ok = false;
        WId windowId = windowIdStr.toULongLong( &ok, 16 );
        if ( ok )
            return windowId;
    }

    return 0;
}
#endif
