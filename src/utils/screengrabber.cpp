
#include "screengrabber.h"
#include "abstractlogger.h"
#include "monitorpreview.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QApplication>
#include <QEventLoop>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include <QWidget>
#include <algorithm>

#ifdef FLAMESHOT_DEBUG_CAPTURE
#include <QDebug>
#endif

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include "request.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QUrl>
#include <QUuid>
#endif

bool ScreenGrabber::m_monitorSelectionActive = false;

ScreenGrabber::ScreenGrabber(QObject* parent)
  : QObject(parent)
  , m_selectedMonitor(-1)
  , m_monitorSelectionLoop(nullptr)
  , m_userCancelled(false)
{
    // Increase image allocation limit for large screenshots
    // (multi-monitor/high-DPI) Default is 128MB, set to 1GB to handle 4K+
    // multi-monitor setups
    QImageReader::setAllocationLimit(1024);
}

void ScreenGrabber::freeDesktopPortal(bool& ok, QPixmap& res)
{

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    auto* connectionInterface = QDBusConnection::sessionBus().interface();
    auto service = QStringLiteral("org.freedesktop.portal.Desktop");

    if (!connectionInterface->isServiceRegistered(service)) {
        ok = false;
        AbstractLogger::error() << tr(
          "Could not locate the `org.freedesktop.portal.Desktop` service");
        return;
    }

    QDBusInterface screenshotInterface(
      service,
      QStringLiteral("/org/freedesktop/portal/desktop"),
      QStringLiteral("org.freedesktop.portal.Screenshot"));

    // unique token
    QString token =
      QUuid::createUuid().toString().remove('-').remove('{').remove('}');

    // premake interface
    auto* request = new OrgFreedesktopPortalRequestInterface(
      service,
      "/org/freedesktop/portal/desktop/request/" +
        QDBusConnection::sessionBus().baseService().remove(':').replace('.',
                                                                        '_') +
        "/" + token,
      QDBusConnection::sessionBus(),
      this);

    QEventLoop loop;

    const auto onPortalResponse = [&res, &loop, this](uint status,
                                                      const QVariantMap& map) {
        if (status == 0) {
            // Parse this as URI to handle unicode properly
            QUrl uri = map.value("uri").toString();
            QString uriString = uri.toLocalFile();
            res = QPixmap(uriString);
            QFile imgFile(uriString);
            imgFile.remove();
        }
        loop.quit();
    };

    // prevent racy situations and listen before calling screenshot
    QMetaObject::Connection conn = QObject::connect(
      request, &org::freedesktop::portal::Request::Response, onPortalResponse);

    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(30000); // 30 second timeout
    QObject::connect(&timeout, &QTimer::timeout, &loop, [&loop, this]() {
        AbstractLogger::error()
          << tr("Screenshot portal timed out after 30 seconds");
        loop.quit();
    });
    timeout.start();

    screenshotInterface.call(
      QStringLiteral("Screenshot"),
      "",
      QMap<QString, QVariant>({ { "handle_token", QVariant(token) },
                                { "interactive", QVariant(false) } }));

    loop.exec();
    timeout.stop();
    QObject::disconnect(conn);
    request->Close().waitForFinished();
    request->deleteLater();

    if (res.isNull()) {
        ok = false;
        return;
    }

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("FreeDesktop portal screenshot size: %1x%2, DPR: %3")
                  .arg(res.width())
                  .arg(res.height())
                  .arg(res.devicePixelRatio());
#endif
#endif
}

QPixmap ScreenGrabber::selectMonitorAndCrop(const QPixmap& fullScreenshot,
                                            bool& ok)
{
    ok = true;
#if defined(Q_OS_MACOS)
    // Avoid showing additional top-level monitor selection UI on macOS
    // Only screenshot the monitor where the tray activated the screenshot
    return cropToMonitor(fullScreenshot, 0);
#else
    // If there's only one monitor, skip selection
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.size() == 1) {
        return cropToMonitor(fullScreenshot, 0);
    }

    if (m_monitorSelectionActive) {
        AbstractLogger::error()
          << tr("Screenshot already in progress, please wait for the current "
                "screenshot to complete");
        ok = false;
        return QPixmap();
    }

    m_monitorSelectionActive = true;
    m_selectedMonitor = -1;
    m_userCancelled = false;
    QWidget* container = createMonitorPreviews(fullScreenshot);

    // Wait for user to select a monitor
    QEventLoop loop;
    m_monitorSelectionLoop = &loop;
    loop.exec();
    m_monitorSelectionLoop = nullptr;

    delete container;
    m_monitorSelectionActive = false;

    if (m_selectedMonitor >= 0) {
        return cropToMonitor(fullScreenshot, m_selectedMonitor);
    } else {
        ok = false;
        if (m_userCancelled) {
            AbstractLogger::info() << tr("Screenshot cancelled");
        }
        return fullScreenshot;
    }
#endif
}

QPixmap ScreenGrabber::grabEntireDesktop(bool& ok, int preSelectedMonitor)
{
    ok = true;
    int wid = 0;
    QPixmap screenshot;

#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (!currentScreen) {
        AbstractLogger::error() << tr("Unable to get current screen");
        ok = false;
        return QPixmap();
    }
    const QRect geom = currentScreen->geometry();
    screenshot = currentScreen->grabWindow(
      wid, geom.x(), geom.y(), geom.width(), geom.height());
    screenshot.setDevicePixelRatio(currentScreen->devicePixelRatio());
    return screenshot;

#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    freeDesktopPortal(ok, screenshot);
    if (!ok) {
        AbstractLogger::error() << tr("Unable to capture screen");
        return QPixmap();
    }

#elif defined(Q_OS_WIN)
    screenshot = windowsScreenshot(wid);
#endif

    // If monitor was pre-selected skip UI and crop directly
    if (preSelectedMonitor >= 0) {
        const QList<QScreen*> screens = QGuiApplication::screens();
        if (preSelectedMonitor < screens.size()) {
            m_selectedMonitor = preSelectedMonitor;
            return cropToMonitor(screenshot, preSelectedMonitor);
        }
    }

    return selectMonitorAndCrop(screenshot, ok);
}

QRect ScreenGrabber::screenGeometry(QScreen* screen)
{
    QRect geometry = screen->geometry();
    if (m_info.waylandDetected()) {
        QPoint topLeft(0, 0);
        geometry.moveTo(geometry.topLeft() - topLeft);
    }
    return geometry;
}

QPixmap ScreenGrabber::grabScreen(QScreen* screen, bool& ok)
{
    QPixmap p;
    QRect geometry = screenGeometry(screen);
#if defined(Q_OS_LINUX)
    p = grabEntireDesktop(ok);
    if (ok) {
        // Both X11 and Wayland: Use cropToMonitor for consistent handling
        // of misaligned monitors and mixed DPI
        const QList<QScreen*> screens = QGuiApplication::screens();
        int screenIndex = screens.indexOf(screen);

        return cropToMonitor(p, screenIndex);
    }
#else
    ok = true;
    return screen->grabWindow(
      0, geometry.x(), geometry.y(), geometry.width(), geometry.height());
#endif
    return p;
}

QRect ScreenGrabber::desktopGeometry()
{
    QRect geometry;

    for (QScreen* const screen : QGuiApplication::screens()) {
        QRect scrRect = screen->geometry();
#if !defined(Q_OS_WIN)
        // https://doc.qt.io/qt-6/highdpi.html#device-independent-screen-geometry
        qreal dpr = screen->devicePixelRatio();
        scrRect.moveTo(QPointF(scrRect.x() / dpr, scrRect.y() / dpr).toPoint());
#endif
        geometry = geometry.united(scrRect);
    }
    return geometry;
}

QScreen* ScreenGrabber::getSelectedScreen() const
{
    const QList<QScreen*> screens = QGuiApplication::screens();

    if ((m_selectedMonitor < 0) || (m_selectedMonitor >= screens.size())) {
        return nullptr;
    }

    return screens[m_selectedMonitor];
}

QWidget* ScreenGrabber::createMonitorPreviews(const QPixmap& fullScreenshot)
{
    const QList<QScreen*> screens = QGuiApplication::screens();

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("=== All Screen Information ===");
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* s = screens[i];
        qDebug() << tr("Screen %1: %2").arg(i).arg(s->name());
        qDebug() << tr("  Logical geometry: %1x%2+%3+%4")
                      .arg(s->geometry().width())
                      .arg(s->geometry().height())
                      .arg(s->geometry().x())
                      .arg(s->geometry().y());
        qDebug() << tr("  DPR: %1").arg(s->devicePixelRatio());
    }
#endif

    QWidget* monitorPreviews = new QWidget(
      nullptr, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    monitorPreviews->setAttribute(Qt::WA_TranslucentBackground);
    monitorPreviews->setStyleSheet(
      "QWidget { background-color: transparent; }");
    monitorPreviews->installEventFilter(this); // For ESC key handling
    monitorPreviews->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout* containerLayout = new QHBoxLayout(monitorPreviews);
    containerLayout->setSpacing(20);
    containerLayout->setContentsMargins(20, 20, 20, 20);

    // Build list of screen indices sorted by X position (left to right)
    QList<int> sortedIndices;
    for (int i = 0; i < screens.size(); ++i) {
        sortedIndices.append(i);
    }
    std::sort(
      sortedIndices.begin(), sortedIndices.end(), [&screens](int a, int b) {
          return screens[a]->geometry().x() < screens[b]->geometry().x();
      });

    for (int i : sortedIndices) {
        QScreen* screen = screens[i];

        QPixmap cropped = cropToMonitor(fullScreenshot, i);
        QPixmap thumbnail = cropped.scaled(
          400, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        thumbnail.setDevicePixelRatio(1.0);

        MonitorPreview* preview =
          new MonitorPreview(i, screen, thumbnail, monitorPreviews);

        connect(
          preview, &MonitorPreview::monitorSelected, this, [this](int index) {
              m_selectedMonitor = index;
              if (m_monitorSelectionLoop) {
                  m_monitorSelectionLoop->quit();
              }
          });

        containerLayout->addWidget(preview);
    }

    monitorPreviews->setLayout(containerLayout);
    monitorPreviews->adjustSize();

    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();
    QPoint center = screenGeometry.center();
    monitorPreviews->move(center.x() - monitorPreviews->width() / 2,
                          center.y() - monitorPreviews->height() / 2);

    monitorPreviews->show();
    return monitorPreviews;
}

bool ScreenGrabber::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            // User cancelled selection
            m_selectedMonitor = -1;
            m_userCancelled = true;
            if (m_monitorSelectionLoop) {
                m_monitorSelectionLoop->quit();
            }
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}

QPixmap ScreenGrabber::cropToMonitor(const QPixmap& fullScreenshot,
                                     int monitorIndex)
{
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (monitorIndex >= screens.size()) {
        return fullScreenshot;
    }

    QScreen* targetScreen = screens[monitorIndex];
    QRect targetGeometry = targetScreen->geometry();
    qreal targetDpr = targetScreen->devicePixelRatio();

    // Calculate total logical dimensions and minimum coordinates
    int minX = 0, minY = 0;
    int maxX = 0, maxY = 0;

    for (QScreen* screen : screens) {
        QRect geo = screen->geometry();
        minX = qMin(minX, geo.x());
        minY = qMin(minY, geo.y());
        maxX = qMax(maxX, geo.x() + geo.width());
        maxY = qMax(maxY, geo.y() + geo.height());
    }

    int totalLogicalWidth = maxX - minX;
    int totalLogicalHeight = maxY - minY;

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("Total logical dimensions: %1x%2 (min: %3,%4)")
                  .arg(totalLogicalWidth)
                  .arg(totalLogicalHeight)
                  .arg(minX)
                  .arg(minY);
    qDebug() << tr("Screenshot dimensions: %1x%2")
                  .arg(fullScreenshot.width())
                  .arg(fullScreenshot.height());
#endif

    int cropX, cropY, cropWidth, cropHeight;

#if defined(Q_OS_LINUX)
    // Linux (both X11 and Wayland via freedesktop portal):
    // Use logical coordinate-based cropping since portal returns full
    // desktop
    qreal screenshotScaleX = (qreal)fullScreenshot.width() / totalLogicalWidth;
    qreal screenshotScaleY =
      (qreal)fullScreenshot.height() / totalLogicalHeight;

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("Screenshot scale factors: X=%1 Y=%2")
                  .arg(screenshotScaleX)
                  .arg(screenshotScaleY);
#endif

    cropX = qRound((targetGeometry.x() - minX) * screenshotScaleX);
    cropY = qRound((targetGeometry.y() - minY) * screenshotScaleY);
    cropWidth = qRound(targetGeometry.width() * screenshotScaleX);
    cropHeight = qRound(targetGeometry.height() * screenshotScaleY);
#else
    // Windows: Calculate physical pixel positions for mixed DPI
    cropX = 0;
    cropY = 0;

    for (QScreen* screen : screens) {
        QRect geom = screen->geometry();
        qreal dpr = screen->devicePixelRatio();

        // Sum physical widths of screens completely to the left
        if (geom.x() + geom.width() <= targetGeometry.x()) {
            cropX += qRound(geom.width() * dpr);
        }

        // Sum physical heights of screens completely above
        if (geom.y() + geom.height() <= targetGeometry.y()) {
            cropY += qRound(geom.height() * dpr);
        }
    }

    cropWidth = qRound(targetGeometry.width() * targetDpr);
    cropHeight = qRound(targetGeometry.height() * targetDpr);

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("Calculated crop position for mixed DPI: X=%1 Y=%2")
                  .arg(cropX)
                  .arg(cropY);
#endif
#endif

    QRect cropRect(cropX, cropY, cropWidth, cropHeight);

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("Screen %1: %2").arg(monitorIndex).arg(targetScreen->name());
    qDebug() << tr("  Logical geometry: %1x%2+%3+%4 DPR: %5")
                  .arg(targetGeometry.width())
                  .arg(targetGeometry.height())
                  .arg(targetGeometry.x())
                  .arg(targetGeometry.y())
                  .arg(targetDpr);
    qDebug() << tr("  Crop rect in screenshot: %1x%2+%3+%4")
                  .arg(cropRect.width())
                  .arg(cropRect.height())
                  .arg(cropRect.x())
                  .arg(cropRect.y());
#endif

    // Ensure crop rect is within bounds
    cropRect = cropRect.intersected(
      QRect(0, 0, fullScreenshot.width(), fullScreenshot.height()));

    if (cropRect.isEmpty()) {
        AbstractLogger::warning()
          << tr("Crop rect is empty, returning full screenshot");
        return fullScreenshot;
    }

    QPixmap cropped = fullScreenshot.copy(cropRect);

#if defined(Q_OS_LINUX)
    // Linux: May need rescaling if scale factors don't match
    if (qAbs(screenshotScaleX - targetDpr) > 0.01) {
        int targetPhysicalWidth = qRound(targetGeometry.width() * targetDpr);
        int targetPhysicalHeight = qRound(targetGeometry.height() * targetDpr);
        cropped = cropped.scaled(targetPhysicalWidth,
                                 targetPhysicalHeight,
                                 Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);
#ifdef FLAMESHOT_DEBUG_CAPTURE
        qDebug() << tr("Scaling screenshot to: %1 %2")
                      .arg(targetPhysicalWidth)
                      .arg(targetPhysicalHeight);
#endif
    }
#endif
    // Cropped region should be at target monitor's native DPR
    cropped.setDevicePixelRatio(targetDpr);

    return cropped;
}

QPixmap ScreenGrabber::windowsScreenshot(int wid)
{
    const QList<QScreen*> screens = QGuiApplication::screens();
    QRect geometry = desktopGeometry();

    int canvasWidth = 0;
    int canvasHeight = 0;

    // Build a map tracking where each screen should be positioned in
    // physical pixels
    struct ScreenInfo
    {
        QRect physicalRect; // Where to draw in the canvas
        QPixmap pixmap;
    };
    QMap<QScreen*, ScreenInfo> screenInfos;

    int minLogicalX = geometry.x();
    int minLogicalY = geometry.y();

    for (QScreen* screen : screens) {
        QRect screenGeom = screen->geometry();
        qreal screenDpr = screen->devicePixelRatio();

        QPixmap screenPixmap = screen->grabWindow(wid);
        screenPixmap.setDevicePixelRatio(1.0);

        int logicalX = screenGeom.x() - minLogicalX;
        int logicalY = screenGeom.y() - minLogicalY;

        int physicalWidth = screenPixmap.width();
        int physicalHeight = screenPixmap.height();

        int physicalX = 0;
        int physicalY = 0;

        for (QScreen* otherScreen : screens) {
            QRect otherGeom = otherScreen->geometry();
            qreal otherDpr = otherScreen->devicePixelRatio();

            // If this screen is entirely to the left of current screen
            if (otherGeom.x() + otherGeom.width() <= screenGeom.x()) {
                physicalX += qRound(otherGeom.width() * otherDpr);
            }

            // If this screen is entirely above the current screen
            if (otherGeom.y() + otherGeom.height() <= screenGeom.y()) {
                physicalY += qRound(otherGeom.height() * otherDpr);
            }
        }

        ScreenInfo info;
        info.physicalRect =
          QRect(physicalX, physicalY, physicalWidth, physicalHeight);
        info.pixmap = screenPixmap;
        screenInfos[screen] = info;

        canvasWidth = qMax(canvasWidth, physicalX + physicalWidth);
        canvasHeight = qMax(canvasHeight, physicalY + physicalHeight);
    }

    // Composite all screens onto canvas
    QPixmap desktop(canvasWidth, canvasHeight);
    desktop.fill(Qt::black);

    QPainter painter(&desktop);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    for (QScreen* screen : screens) {
        const ScreenInfo& info = screenInfos[screen];
        painter.drawPixmap(info.physicalRect.topLeft(), info.pixmap);
    }
    painter.end();

    return desktop;
}
