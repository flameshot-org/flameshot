#include "trayicon.h"

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "qcollator.h"
#include "qdir.h"
#include "qmessagebox.h"
#include "qprocess.h"
#include "qregularexpression.h"
#include "qstandardpaths.h"
#include "screenshotsaver.h"
#include "src/core/flameshot.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/globalvalues.h"

#include "src/utils/confighandler.h"
#include <QApplication>
#include <QMenu>
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

#if defined( Q_OS_WIN )
#include <qscreen.h>
#include <windows.h>
#include "tools/windowhighlightoverlay.h"
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
      QIcon::fromTheme("flameshot-tray", QIcon(GlobalValues::iconPathPNG()));
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
            [this]() {});
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

    auto* captureAction = new QAction(tr("&Take Screenshot"), this);
    connect(captureAction, &QAction::triggered, this, [this]() {
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

#if defined( Q_OS_LINUX )
        QMessageBox msgCapturaWithScroll;

        msgCapturaWithScroll.setText( QObject::tr("Select the sale you want to capture with a scroll and leave the mouse over it until you finish and the \"Save Image\" text box appears..." ) );
        msgCapturaWithScroll.exec();

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

        int width = winAttr.width;
        int height = winAttr.height;
#endif
        int xOffset = 0;
        int yOffset = 50;

#if defined( Q_OS_LINUX )

        captureScreenScroll* captureSS = new captureScreenScroll( static_cast<void*>( display ), targetWindowId, xOffset, yOffset, width, height );

#elif defined ( Q_OS_WIN )

        captureScreenScroll* captureSS = new captureScreenScroll();

        static WindowHighlightOverlay overlay;
        overlay.initVirtualDesktop();
        overlay.startTracking();

        static bool scrollEnCurso = false;

        QObject::connect(&overlay, &WindowHighlightOverlay::panelSelected,
                         &overlay,
                         [&, hwnd = HWND{}](HWND selectedHwnd) mutable {
                             if (scrollEnCurso) {
                                 qDebug() << "Scroll ya en curso. Ignorando.";
                                 return;
                             }

                                    // ⚠️ Filtro: ignorar barra de scroll u objetos pequeños
                             RECT rect;
                             if (GetWindowRect(selectedHwnd, &rect)) {
                                 int width = rect.right - rect.left;
                                 int height = rect.bottom - rect.top;

                                 if (width < 80 || height < 80) {
                                     qDebug() << "HWND ignorado por tamaño sospechoso:" << width << "x" << height;
                                     return;
                                 }
                             }

                             scrollEnCurso = true;
                             hwnd = selectedHwnd;

                             QObject::disconnect(&overlay, nullptr, &overlay, nullptr);

                             /*QString baseDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/FlameshotScroll";
                             QDir().mkpath(baseDir);*/

#endif
                             QString picturesPath = QStandardPaths::writableLocation( QStandardPaths::PicturesLocation );
                             QString baseDir = picturesPath + "/FlameshotCapture";
                             QDir().mkpath( baseDir );

                             int shotIdx = 0;
#if defined (Q_OS_WIN)
                                    // Llevar al inicio como ShareX
                             SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0);
                             Sleep(200);
#endif

#if defined( Q_OS_LINUX )
                             QPixmap previousCapture;
#elif defined ( Q_OS_WIN )

                             QImage previousCapture;

#endif

                             bool hasPrevious = false;

                             while ( true ) {
 #if defined ( Q_OS_WIN )

                                QScreen* screen = nullptr;
                                for (QScreen* s : QGuiApplication::screens()) {
                                    if (s->geometry().contains(QCursor::pos())) {
                                        screen = s;
                                        break;
                                    }
                                }

                                if (!screen) screen = QGuiApplication::primaryScreen();

                                QImage currentCapture = captureSS -> captureWithPrintWindow(hwnd);
                                if (currentCapture.isNull()) {

                                    qDebug() << "❌ NULLLL.....";
                                    break;
                                }

                                QImage previousCapture2 = previousCapture;

                                QImage currentCapture2 = currentCapture;

#endif
#if defined ( Q_OS_LINUX )
                                QPixmap currentCapture = captureSS -> captureScrollableArea();

                                if ( currentCapture.isNull() ) {
                                    qDebug() << "❌ Captura nula. Terminando.";
                                    break;
                                }

                                QImage previousCapture2 = previousCapture.toImage();

                                QImage currentCapture2 = currentCapture.toImage();

#endif
                                //qDebug() << "captureSS -> imagesEqual( previousCapture2, currentCapture2 ): " << captureSS -> imagesEqual( previousCapture2, currentCapture2 );

                                /*if ( captureSS -> imagesEqual( previousCapture2, currentCapture2 ) ) {
                                    qDebug() << "⚠️ Imagen no cambió. Deteniendo scroll.";
                                    break;
                                }*/

                                if (hasPrevious) {
                                    bool iguales = captureSS->imagesEqual(previousCapture2, currentCapture2);
                                    qDebug() << "imagesEqual(prev, curr):" << iguales;

                                    if (iguales) {
                                        qDebug() << "⚠️ Imagen no cambió. Deteniendo scroll.";
                                        break;
                                    }
                                } else {
                                    qDebug() << "Primera captura, no se compara aún.";
                                }

                                QString const filename = QString( "captura%1.png" ).arg(shotIdx++, 3, 10, QChar('0'));

                                currentCapture.save( baseDir + "/" + filename );

                                qDebug() << "📸 Captura guardada:" << filename;

                                previousCapture = currentCapture;
                                hasPrevious = true;

#if defined ( Q_OS_LINUX )
                                // Scroll hacia abajo
                                //XSetInputFocus( display, targetWindowId, RevertToParent, CurrentTime );  // Asegura foco
                                XTestFakeButtonEvent( display, 5, True, CurrentTime );
                                XFlush( display );
                                XTestFakeButtonEvent( display, 5, False, CurrentTime );
                                XFlush( display );
                                usleep( 200000 );
#endif

#if defined (Q_OS_WIN)
                                       // Scroll hacia abajo
                                INPUT scroll = {};
                                scroll.type = INPUT_MOUSE;
                                scroll.mi.dwFlags = MOUSEEVENTF_WHEEL;
                                scroll.mi.mouseData = -3 * WHEEL_DELTA;

                                SendInput(1, &scroll, sizeof(INPUT));

                                //std::this_thread::sleep_for(std::chrono::milliseconds(200));

                                QThread::msleep(1000);
#endif

                            }

#if defined ( Q_OS_WIN )
                            overlay.stopTracking();
                            overlay.hide();
                            scrollEnCurso = false;
#endif

                            // Post-procesamiento
                            struct Item { std::string path; std::string name; };
                            std::vector<Item> items;

                            for (const auto& e : fs::directory_iterator( baseDir.toStdString() ) )
                                if ( e.is_regular_file() && e.path().extension() == ".png" )
                                    items.push_back( { e.path().string(), e.path().filename().string() } );

                            /*std::stable_sort(items.begin(), items.end(),
                                      [](const Item& x, const Item& y) {
                                          return x.name < y.name;
                                      });*/

                            QCollator coll;
                            coll.setNumericMode( true );
                            coll.setCaseSensitivity( Qt::CaseInsensitive );

                            std::stable_sort( items.begin(), items.end(),
                                             [ & ]( const Item& a, const Item& b ) {
                                                 // a.name y b.name son std::string en UTF-8
                                                 return coll.compare( QString::fromUtf8( a.name ),
                                                                      QString::fromUtf8( b.name ) ) < 0;
                                             } );

                            std::vector<std::string> paths;
                            for ( const auto& i : items )
                                paths.push_back( i.path );

                            /*cv::Mat result;
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
                            }*/

                            /*****************/

                            cv::Mat result;
                            cv::Mat firstRefImage;

                            for ( const auto& f : paths ) {
                                qDebug() << "procesando " << QString::fromStdString( f ) << '\n';
                                cv::Mat img = cv::imread( f );
                                if ( img.empty() ) {
                                    std::cerr << "No se lee " << f << '\n';
                                    continue;
                                }

                                cv::Mat cropped = captureSS->cropHorizontal(img);

                                qDebug() << "img size:"     << img.cols << "x" << img.rows
                                         << "cropped size:" << cropped.cols << "x" << cropped.rows;

                                if (cropped.empty()) {
                                    qDebug() << "⚠️ cropped vacío, se omite esta imagen";
                                    continue;
                                }

                                if ( result.empty() ) {
                                    result = cropped.clone();
                                    firstRefImage = cropped.clone();
                                    qDebug() << "result inicializado con size:"
                                             << result.cols << "x" << result.rows;
                                    continue;
                                }

                                qDebug() << "Antes de combineImages:"
                                         << "result:"  << result.cols  << "x" << result.rows
                                         << "cropped:" << cropped.cols << "x" << cropped.rows;

                                       // aquí viene el dolor 💥
                                result = captureSS->combineImages(result, cropped);
                            }


                            /*****************/

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

    } );

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

    m_menu->addAction(captureAction);
    m_menu -> addAction( captureActionWithDesplazamiento );
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
    m_menu->addAction(infoAction);
    m_menu->addSeparator();
    m_menu->addAction(quitAction);
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
    auto* widget = Flameshot::instance()->gui();
#if !defined(DISABLE_UPDATE_CHECKER)
    FlameshotDaemon::instance()->showUpdateNotificationIfAvailable(widget);
#endif
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
