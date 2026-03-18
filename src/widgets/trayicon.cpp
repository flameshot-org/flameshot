#include "trayicon.h"

#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include "qcollator.h"
#include "qdir.h"
#include "qmessagebox.h"
#include "qstandardpaths.h"
#include "screenshotsaver.h"
#include "src/core/flameshot.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/globalvalues.h"
#include "src/utils/confighandler.h"

#include <QApplication>
#include <QMenu>
#include <QTimer>
#include <QVersionNumber>
#include <QThread>
#include <QCursor>
#include <QEventLoop>
#include <QPointer>
#include <QScreen>
#include <QWindow>
#include <QList>
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>

#if defined(Q_OS_MACOS)
#include <QOperatingSystemVersion>
#endif

#include <capturescreenscroll.h>

#if defined(Q_OS_LINUX)
#include "src/tools/overlay/scrollregionselector.h"
#include "src/tools/uinputscrollcontroller.h"
#include "tools/wayland/waylandportalcapturebackend.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#include "tools/windowhighlightoverlay.h"
#endif

namespace {

struct Item {
    std::string path;
    std::string name;
};

std::vector<std::string> collectSortedPngs(const QString& baseDir)
{
    std::vector<Item> items;

    for (const auto& e : fs::directory_iterator(baseDir.toStdString())) {
        if (e.is_regular_file() && e.path().extension() == ".png") {
            items.push_back({ e.path().string(), e.path().filename().string() });
        }
    }

    QCollator coll;
    coll.setNumericMode(true);
    coll.setCaseSensitivity(Qt::CaseInsensitive);

    std::stable_sort(items.begin(),
                     items.end(),
                     [&](const Item& a, const Item& b) {
                         return coll.compare(QString::fromUtf8(a.name),
                                             QString::fromUtf8(b.name)) < 0;
                     });

    std::vector<std::string> paths;
    paths.reserve(items.size());
    for (const auto& i : items) {
        paths.push_back(i.path);
    }

    return paths;
}

#if defined(Q_OS_LINUX)
QRect selectScrollRegion()
{
    QEventLoop loop;
    QRect selected;
    bool accepted = false;

    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        qWarning() << "No se encontraron pantallas.";
        return QRect();
    }

    QRect virtualGeometry;
    for (QScreen* s : screens) {
        qDebug() << "Pantalla X11:" << s->name() << s->geometry();
        virtualGeometry = virtualGeometry.united(s->geometry());
    }

    if (!virtualGeometry.isValid()) {
        qWarning() << "Geometría virtual inválida.";
        return QRect();
    }

    auto* selector = new ScrollRegionSelector();
    selector->setAttribute(Qt::WA_DeleteOnClose, false);
    selector->setGeometry(virtualGeometry);

    selector->winId();

    qDebug() << "Creando selector X11 geometry =" << selector->geometry();

    QObject::connect(selector,
                     &ScrollRegionSelector::selectionFinished,
                     [&loop, &selected, &accepted, selector, virtualGeometry](const QRect& rect) {
                         selected = QRect(
                           virtualGeometry.x() + rect.x(),
                           virtualGeometry.y() + rect.y(),
                           rect.width(),
                           rect.height());
                         accepted = true;

                         if (selector) {
                             selector->hide();
                             selector->close();
                         }

                         QApplication::processEvents();
                         loop.quit();
                     });

    QObject::connect(selector,
                     &ScrollRegionSelector::selectionCanceled,
                     [&loop, &accepted, selector]() {
                         accepted = false;

                         if (selector) {
                             selector->hide();
                             selector->close();
                         }

                         QApplication::processEvents();
                         loop.quit();
                     });

    QTimer::singleShot(10, [selector]() {
        if (!selector) {
            return;
        }

        selector->show();
        selector->raise();
        selector->activateWindow();
        selector->setFocus(Qt::ActiveWindowFocusReason);
        QApplication::processEvents();
    });

    loop.exec();

    if (selector) {
        selector->deleteLater();
    }

    QApplication::processEvents();
    return accepted ? selected : QRect();
}

QPoint currentPointerPos(Display* display)
{
    if (!display) {
        return QPoint();
    }

    Window root = DefaultRootWindow(display);
    Window rootReturn, childReturn;
    int rootX = 0, rootY = 0, winX = 0, winY = 0;
    unsigned int mask = 0;

    if (XQueryPointer(display,
                      root,
                      &rootReturn,
                      &childReturn,
                      &rootX,
                      &rootY,
                      &winX,
                      &winY,
                      &mask)) {
        return QPoint(rootX, rootY);
    }

    return QPoint();
}

bool movePointerTo(Display* display, const QPoint& globalPos)
{
    if (!display) {
        return false;
    }

    Window root = DefaultRootWindow(display);
    XWarpPointer(display, None, root, 0, 0, 0, 0, globalPos.x(), globalPos.y());
    XFlush(display);
    return true;
}
#endif

} // namespace

TrayIcon::TrayIcon(QObject* parent)
  : QSystemTrayIcon(parent)
{
    initMenu();

#if defined(Q_OS_LINUX)
    m_waylandBackend = new WaylandPortalCaptureBackend(this);
#endif
    setToolTip(QStringLiteral("Flameshot"));

#if defined(Q_OS_MACOS)
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
        auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason) {
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
    connect(qApp, &QCoreApplication::aboutToQuit, this, &TrayIcon::hide);
#endif

    show();

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

QString TrayIcon::createScrollCaptureDir() const
{
    QString picturesPath =
      QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString baseDir = picturesPath + "/FlameshotCapture";
    QDir().mkpath(baseDir);
    return baseDir;
}

bool TrayIcon::runCaptureLoop(ScrollCaptureContext& ctx) const
{
    bool capturedAny = false;
    int sameCount = 0;

    while (true) {
        QImage currentCapture = ctx.grabFrame();

        if (currentCapture.isNull()) {
            qDebug() << "❌ Captura nula. Terminando.";
            break;
        }

        bool iguales = false;

        if (ctx.hasPrevious) {
            if (ctx.captureSS) {
                iguales = ctx.captureSS->imagesEqual(ctx.previousCapture, currentCapture);
            } else {
                iguales = (ctx.previousCapture == currentCapture);
            }

            qDebug() << "imagesEqual(prev, curr):" << iguales;
        } else {
            qDebug() << "Primera captura, no se compara aún.";
        }

        if (ctx.hasPrevious && iguales) {
            sameCount++;
            qDebug() << "⚠️ Imagen sin cambio. Conteo:" << sameCount;

            if (sameCount >= 3) {
                qDebug() << "⛔ Tres capturas iguales consecutivas. Deteniendo scroll.";
                break;
            }

            QImage beforeScroll = currentCapture;

            if (!ctx.doScroll()) {
                break;
            }

#if defined(Q_OS_LINUX)
            waitAfterScroll(beforeScroll);
#endif
            continue;
        }

        sameCount = 0;

        const QString filename =
          QString("captura%1.png").arg(ctx.shotIdx++, 3, 10, QChar('0'));

        if (!currentCapture.save(ctx.baseDir + "/" + filename)) {
            qDebug() << "❌ No se pudo guardar la captura:" << filename;
            break;
        }

        qDebug() << "📸 Captura guardada:" << filename
                 << "size =" << currentCapture.size();

        capturedAny = true;

        ctx.previousCapture = currentCapture;
        ctx.hasPrevious = true;

        QImage beforeScroll = currentCapture;

        if (!ctx.doScroll()) {
            break;
        }

#if defined(Q_OS_LINUX)
        waitAfterScroll(beforeScroll);
#endif
    }

    return capturedAny;
}

#if defined(Q_OS_LINUX)
void TrayIcon::waitAfterScroll(const QImage& beforeScroll) const
{
    if (m_waylandBackend && m_waylandBackend->isReady()) {
        QThread::msleep(250);
        m_waylandBackend->waitAfterExternalScroll(beforeScroll, 1500);
    } else {
        QThread::msleep(150);
    }
}
#endif

bool TrayIcon::stitchAndSaveResult(captureScreenScroll* captureSS,
                                   const QString& baseDir) const
{
    const std::vector<std::string> paths = collectSortedPngs(baseDir);

    std::vector<cv::Mat> validImages;
    validImages.reserve(paths.size());

    for (const auto& f : paths) {
        qDebug() << "procesando " << QString::fromStdString(f) << '\n';

        cv::Mat img = cv::imread(f);
        if (img.empty()) {
            std::cerr << "No se lee " << f << '\n';
            continue;
        }

        cv::Mat cropped = captureSS->cropHorizontal(img);

        qDebug() << "img size:" << img.cols << "x" << img.rows
                 << "cropped size:" << cropped.cols << "x" << cropped.rows;

        if (cropped.empty()) {
            qDebug() << "⚠️ cropped vacío, se omite esta imagen";
            continue;
        }

        validImages.push_back(cropped);
    }

    if (validImages.empty()) {
        qDebug() << "❌ No hay imágenes válidas para stitching.";
        return false;
    }

    cv::Mat result = validImages[0].clone();

    qDebug() << "result inicializado con size:"
             << result.cols << "x" << result.rows;

    for (size_t i = 1; i < validImages.size(); ++i) {
        const cv::Mat& cropped = validImages[i];
        const bool isLastImage = (i == validImages.size() - 1);

        qDebug() << "Antes de combineImages:"
                 << "result:" << result.cols << "x" << result.rows
                 << "cropped:" << cropped.cols << "x" << cropped.rows
                 << "isLastImage =" << isLastImage;

        cv::Mat combined = captureSS->combineImages(result, cropped, isLastImage);
        if (combined.empty()) {
            qDebug() << "⚠️ No se pudo combinar la captura actual. Terminando stitching.";
            break;
        }

        result = combined;
    }

    if (result.empty()) {
        qDebug() << "❌ Stitching vacío. No se generó imagen final.";
        return false;
    }

    QPixmap finalImage = captureSS->cvMatToQPixmap(result);
    if (finalImage.isNull()) {
        qDebug() << "Failed to stitch images.";
        return false;
    }

    cv::imwrite(baseDir.toStdString() + "/resultado_stitched.png", result);
    saveToFilesystemGUI(finalImage);

    QString dirPath = baseDir;
    QDir(dirPath).removeRecursively();
    return true;
}

void TrayIcon::startScrollingCapture()
{
    ScrollCaptureContext ctx;
    ctx.baseDir = createScrollCaptureDir();

#if defined(Q_OS_LINUX)
    if (!setupLinuxScrollingCapture(ctx)) {
        return;
    }
#elif defined(Q_OS_WIN)
    if (!setupWindowsScrollingCapture(ctx)) {
        return;
    }
#elif defined(Q_OS_MACOS)
    QMessageBox::information(
      nullptr,
      QObject::tr("Scrolling screenshot"),
      QObject::tr("La captura con desplazamiento aún no está implementada para macOS."));
    return;
#endif

    const bool captured = runCaptureLoop(ctx);

    if (ctx.cleanup) {
        ctx.cleanup();
    }

    if (captured && ctx.captureSS) {
        if (!stitchAndSaveResult(ctx.captureSS, ctx.baseDir)) {
            qDebug() << "❌ Falló el stitching final.";
        }
    } else {
        qDebug() << "⚠️ No hubo capturas válidas. Se omite stitching.";
    }

    delete ctx.captureSS;
    ctx.captureSS = nullptr;
}

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
            QTimer::singleShot(400, this, [this]() { startGuiCapture(); });
        }
#else
        QTimer::singleShot(400, this, [this]() { startGuiCapture(); });
#endif
    });

    auto* captureActionWithDesplazamiento =
      new QAction(tr("&Scrolling screenshot"), this);
    connect(captureActionWithDesplazamiento,
            &QAction::triggered,
            this,
            [this]() { startScrollingCapture(); });

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
    connect(infoAction,
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
                const QString newVersion =
                  tr("New version %1 is available").arg(version.toString());
                m_appUpdates->setText(newVersion);
            });
#endif

    QAction* quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

#ifdef ENABLE_IMGUR
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
    m_menu->addAction(captureActionWithDesplazamiento);
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

#if defined(Q_OS_LINUX)
QRect TrayIcon::selectScrollRegionFromImage(const QImage& screenshot)
{
    if (screenshot.isNull()) {
        return QRect();
    }

    QEventLoop loop;
    QRect selected;
    bool accepted = false;

    auto* selector = new ScrollRegionSelector();
    selector->setAttribute(Qt::WA_DeleteOnClose, false);
    selector->setScreenshot(screenshot);
    selector->setGeometry(0, 0, screenshot.width(), screenshot.height());

    QObject::connect(selector,
                     &ScrollRegionSelector::selectionFinished,
                     [&loop, &selected, &accepted, selector](const QRect& rect) {
                         selected = rect;
                         accepted = true;
                         if (selector) {
                             selector->hide();
                             selector->close();
                         }
                         QApplication::processEvents();
                         loop.quit();
                     });

    QObject::connect(selector,
                     &ScrollRegionSelector::selectionCanceled,
                     [&loop, &accepted, selector]() {
                         accepted = false;
                         if (selector) {
                             selector->hide();
                             selector->close();
                         }
                         QApplication::processEvents();
                         loop.quit();
                     });

    QTimer::singleShot(10, [selector]() {
        if (!selector) {
            return;
        }

        selector->show();
        selector->raise();
        selector->activateWindow();
        selector->setFocus(Qt::ActiveWindowFocusReason);
        QApplication::processEvents();
    });

    loop.exec();

    if (selector) {
        selector->deleteLater();
    }

    QApplication::processEvents();
    return accepted ? selected : QRect();
}

bool TrayIcon::setupLinuxScrollingCapture(ScrollCaptureContext& ctx)
{
    const QByteArray waylandDisplay = qgetenv("WAYLAND_DISPLAY");
    const QByteArray sessionType = qgetenv("XDG_SESSION_TYPE").toLower();
    const bool isWayland = !waylandDisplay.isEmpty() || sessionType == "wayland";

    /*if (isWayland) {
        WaylandPortalCaptureBackend* backend = m_waylandBackend;

        if (!backend) {
            qWarning() << "m_waylandBackend es null";
            return false;
        }

        if (!backend->initialize(QString())) {
            QMessageBox::warning(
              nullptr,
              QObject::tr("Scrolling screenshot"),
              QObject::tr(
                "No se pudo inicializar la captura Wayland usando "
                "xdg-desktop-portal + PipeWire."));
            return false;
        }

        QImage firstFrame;
        for (int i = 0; i < 30; ++i) {
            QThread::msleep(100);
            firstFrame = backend->latestFrame();
            if (!firstFrame.isNull()) {
                break;
            }
            QApplication::processEvents();
        }

        if (firstFrame.isNull()) {
            backend->shutdown();
            //delete backend;
            QMessageBox::warning(
              nullptr,
              QObject::tr("Scrolling screenshot"),
              QObject::tr("No se pudo obtener la imagen inicial de la pantalla en Wayland."));
            return false;
        }

        QRect selectedRect = selectScrollRegionFromImage(firstFrame);
        if (selectedRect.isNull() ||
            selectedRect.width() < 20 ||
            selectedRect.height() < 20) {
            qDebug() << "Selección cancelada o inválida en Wayland.";
            backend->shutdown();
            //delete backend;
            return false;
        }

        qDebug() << "Wayland selectedRect from image =" << selectedRect;

        backend->setSelectedRect(selectedRect);

        QApplication::processEvents();

        QImage frameWithOverlay = backend->latestFrame();

        QThread::msleep(250);
        backend->waitAfterExternalScroll(frameWithOverlay, 1500);

        QApplication::processEvents();
        QThread::msleep(100);

        ctx.captureSS = new captureScreenScroll();

        ctx.grabFrame = [backend]() -> QImage {
            return backend->latestFrame();
        };

        ctx.doScroll = [backend]() -> bool {
            if (!backend->movePointerToSelectedRectCenter()) {
                qDebug() << "❌ No se pudo mover el puntero al centro del rect seleccionado.";
                return false;
            }

            QThread::msleep(120);

            qDebug() << "Enviando scrollDown(3) en Wayland";
            if (!backend->scrollDown(3)) {
                qDebug() << "❌ Error enviando scroll Wayland.";
                return false;
            }

            QThread::msleep(350);
            return true;
        };

        ctx.cleanup = [backend]() {
            backend->shutdown();
            //delete backend;
        };

        return true;
    }*/

    if (isWayland) {
        WaylandPortalCaptureBackend* backend = m_waylandBackend;

        if (!backend) {
            qWarning() << "m_waylandBackend es null";
            return false;
        }

        if (!backend->initialize(QString())) {
            QMessageBox::warning(
              nullptr,
              QObject::tr("Scrolling screenshot"),
              QObject::tr(
                "No se pudo inicializar la captura Wayland usando "
                "xdg-desktop-portal + PipeWire."));
            return false;
        }

        QImage firstFrame;
        for (int i = 0; i < 30; ++i) {
            QThread::msleep(100);
            firstFrame = backend->latestFrame();
            if (!firstFrame.isNull()) {
                break;
            }
            QApplication::processEvents();
        }

        if (firstFrame.isNull()) {
            backend->shutdown();
            QMessageBox::warning(
              nullptr,
              QObject::tr("Scrolling screenshot"),
              QObject::tr("No se pudo obtener la imagen inicial de la pantalla en Wayland."));
            return false;
        }

        QRect selectedRect = selectScrollRegionFromImage(firstFrame);
        if (selectedRect.isNull() ||
            selectedRect.width() < 20 ||
            selectedRect.height() < 20) {
            qDebug() << "Selección cancelada o inválida en Wayland.";
            backend->shutdown();
            return false;
        }

        qDebug() << "Wayland selectedRect from image =" << selectedRect;

        backend->setSelectedRect(selectedRect);

        QApplication::processEvents();

        QImage frameWithOverlay = backend->latestFrame();

        QThread::msleep(250);
        backend->waitAfterExternalScroll(frameWithOverlay, 1500);

        QApplication::processEvents();
        QThread::msleep(100);

        ctx.captureSS = new captureScreenScroll();

        ctx.grabFrame = [backend]() -> QImage {
            //return backend->latestFrame();
            return backend->latestFrame().copy();
        };

        ctx.doScroll = [backend]() -> bool {
            if (!backend->movePointerToSelectedRectCenter()) {
                qDebug() << "❌ No se pudo mover el puntero al centro del rect seleccionado.";
                return false;
            }

            QThread::msleep(120);

            qDebug() << "Enviando scrollDown(3) en Wayland";
            if (!backend->scrollDown(3)) {
                qDebug() << "❌ Error enviando scroll Wayland.";
                return false;
            }

            QThread::msleep(350);
            return true;
        };

        ctx.cleanup = [backend]() {
            backend->shutdown();
        };

        return true;
    }
           // -----------------------------
           // X11 / Xorg: selector transparente global, sin screenshot
           // -----------------------------
    QRect selectedGlobalRect = selectScrollRegion();

    QApplication::processEvents();
    QThread::msleep(80);

    if (selectedGlobalRect.isNull() ||
        selectedGlobalRect.width() < 20 ||
        selectedGlobalRect.height() < 20) {
        qDebug() << "Selección cancelada o inválida.";
        return false;
    }

    qDebug() << "selectedGlobalRect =" << selectedGlobalRect;

    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        qWarning() << "No se pudo abrir X11 Display.";
        return false;
    }

    const QPoint scrollTarget = selectedGlobalRect.center();

    qDebug() << "scrollTarget =" << scrollTarget;

    const int xOffset = selectedGlobalRect.x();
    const int yOffset = selectedGlobalRect.y();
    const int width = selectedGlobalRect.width();
    const int height = selectedGlobalRect.height();

    auto* scrollController = new UInputScrollController();
    if (!scrollController->init()) {
        delete scrollController;
        QMessageBox::warning(
          nullptr,
          QObject::tr("Permisos insuficientes"),
          QObject::tr(
            "No se pudo acceder a /dev/uinput.\n\n"
            "Para habilitar el scroll automático en Linux/X11, "
            "debes dar permisos al dispositivo mediante udev y agregar "
            "tu usuario al grupo correspondiente."));
        XCloseDisplay(display);
        return false;
    }

    ctx.captureSS = new captureScreenScroll(
      static_cast<void*>(display),
      0,
      xOffset,
      yOffset,
      width,
      height);

    ctx.grabFrame = [captureSS = ctx.captureSS]() -> QImage {
        return captureSS->captureScrollableArea().toImage();
    };

    ctx.doScroll = [scrollController, display, scrollTarget]() -> bool {
        const QPoint oldPos = currentPointerPos(display);

        if (!movePointerTo(display, scrollTarget)) {
            qDebug() << "❌ No se pudo mover el puntero al área seleccionada.";
            return false;
        }

        QThread::msleep(80);

        qDebug() << "Enviando scrollDown(3) en" << scrollTarget;
        if (!scrollController->scrollDown(3)) {
            qDebug() << "❌ Error enviando scroll por uinput.";
            movePointerTo(display, oldPos);
            return false;
        }

        QThread::msleep(350);
        movePointerTo(display, oldPos);
        return true;
    };

    ctx.cleanup = [scrollController, display]() {
        scrollController->shutdown();
        delete scrollController;
        XCloseDisplay(display);
    };

    return true;
}
#endif

#if defined(Q_OS_WIN)
bool TrayIcon::setupWindowsScrollingCapture(ScrollCaptureContext& ctx)
{
    ctx.captureSS = new captureScreenScroll();

    static WindowHighlightOverlay overlay;
    overlay.initVirtualDesktop();
    overlay.startTracking();

    static bool scrollEnCurso = false;
    HWND hwnd = nullptr;

    QEventLoop loop;

    QObject::connect(&overlay,
                     &WindowHighlightOverlay::panelSelected,
                     &overlay,
                     [&](HWND selectedHwnd) {
                         if (scrollEnCurso) {
                             qDebug() << "Scroll ya en curso. Ignorando.";
                             return;
                         }

                         RECT rect;
                         if (GetWindowRect(selectedHwnd, &rect)) {
                             const int width = rect.right - rect.left;
                             const int height = rect.bottom - rect.top;

                             if (width < 80 || height < 80) {
                                 qDebug() << "HWND ignorado por tamaño sospechoso:"
                                          << width << "x" << height;
                                 return;
                             }
                         }

                         scrollEnCurso = true;
                         hwnd = selectedHwnd;
                         QObject::disconnect(&overlay, nullptr, &overlay, nullptr);
                         loop.quit();
                     });

    loop.exec();

    if (!hwnd) {
        overlay.stopTracking();
        overlay.hide();
        delete ctx.captureSS;
        ctx.captureSS = nullptr;
        return false;
    }

    SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0);
    Sleep(200);

    ctx.grabFrame = [captureSS = ctx.captureSS, hwnd]() -> QImage {
        return captureSS->captureWithPrintWindow(hwnd);
    };

    ctx.doScroll = []() -> bool {
        INPUT scroll = {};
        scroll.type = INPUT_MOUSE;
        scroll.mi.dwFlags = MOUSEEVENTF_WHEEL;
        scroll.mi.mouseData = -3 * WHEEL_DELTA;
        SendInput(1, &scroll, sizeof(INPUT));
        QThread::msleep(1000);
        return true;
    };

    /*ctx.cleanup = [&overlay, &scrollEnCurso]() {
        overlay.stopTracking();
        overlay.hide();
        scrollEnCurso = false;
    };*/

    ctx.cleanup = []() {
        overlay.stopTracking();
        overlay.hide();
        scrollEnCurso = false;
    };

    return true;
}
#endif

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
