// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

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
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QProcess>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

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

ScreenGrabber::ScreenGrabber(QObject* parent)
  : QObject(parent)
  , m_selectedMonitor(-1)
  , m_monitorSelectionLoop(nullptr)
{}

void ScreenGrabber::generalGrimScreenshot(bool& ok, QPixmap& res)
{
#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    if (!ConfigHandler().useGrimAdapter()) {
        return;
    }

    QString runDir =
      QProcessEnvironment::systemEnvironment().value("XDG_RUNTIME_DIR");
    QString imgPath = runDir + "/flameshot.ppm";
    QProcess Process;
    QString program = "grim";
    QStringList arguments;
    arguments << "-t"
              << "ppm" << imgPath;
    Process.start(program, arguments);
    if (Process.waitForFinished()) {
        res.load(imgPath, "ppm");
        QFile imgFile(imgPath);
        imgFile.remove();
        adjustDevicePixelRatio(res);
        ok = true;
    } else {
        ok = false;
        AbstractLogger::error()
          << tr("The universal wayland screen capture adapter requires Grim as "
                "the screen capture component of wayland. If the screen "
                "capture component is missing, please install it!");
    }
#endif
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
    QPixmap fullScreenshot;

    const auto onPortalResponse =
      [&fullScreenshot, &loop, this](uint status, const QVariantMap& map) {
          if (status == 0) {
              // Parse this as URI to handle unicode properly
              QUrl uri = map.value("uri").toString();
              QString uriString = uri.toLocalFile();
              fullScreenshot = QPixmap(uriString);
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
    timeout.stop(); // Stop timeout if we got a response
    QObject::disconnect(conn);
    request->Close().waitForFinished();
    request->deleteLater();

    if (fullScreenshot.isNull()) {
        ok = false;
        return;
    }

    QPixmap result = selectMonitorAndCrop(fullScreenshot, ok);
    if (ok) {
        res = result;
    }
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

    m_selectedMonitor = -1;
    QWidget* container = createMonitorPreviews(fullScreenshot);

    // Wait for user to select a monitor
    QEventLoop loop;
    m_monitorSelectionLoop = &loop;
    loop.exec();
    m_monitorSelectionLoop = nullptr;

    delete container;

    if (m_selectedMonitor >= 0) {
        return cropToMonitor(fullScreenshot, m_selectedMonitor);
    } else {
        ok = false;
        return fullScreenshot;
    }
#endif
}

QPixmap ScreenGrabber::grabEntireDesktop(bool& ok)
{
    ok = true;
    int wid = 0;

#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (!currentScreen) {
        AbstractLogger::error() << tr("Unable to get current screen");
        ok = false;
        return QPixmap();
    }
    const QRect geom = currentScreen->geometry();
    QPixmap screenPixmap = currentScreen->grabWindow(
      wid, geom.x(), geom.y(), geom.width(), geom.height());
    screenPixmap.setDevicePixelRatio(currentScreen->devicePixelRatio());
    return screenPixmap;
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_info.waylandDetected()) {
        QPixmap res;
        // handle screenshot based on DE
        switch (m_info.windowManager()) {
            case DesktopInfo::GNOME:
            case DesktopInfo::KDE:
            case DesktopInfo::COSMIC:
                freeDesktopPortal(ok, res);
                break;
            case DesktopInfo::QTILE:
            case DesktopInfo::WLROOTS:
            case DesktopInfo::HYPRLAND:
            case DesktopInfo::OTHER: {
                if (!ConfigHandler().useGrimAdapter()) {
                    if (!ConfigHandler().disabledGrimWarning()) {
                        AbstractLogger::warning() << tr(
                          "If the useGrimAdapter setting is not enabled, the "
                          "dbus protocol will be used. It should be noted that "
                          "using the dbus protocol under wayland is not "
                          "recommended. It is recommended to enable the "
                          "useGrimAdapter setting in flameshot.ini to activate "
                          "the grim-based general wayland screenshot adapter");
                    }
                    freeDesktopPortal(ok, res);
                } else {
                    if (!ConfigHandler().disabledGrimWarning()) {
                        AbstractLogger::warning() << tr(
                          "grim's screenshot component is implemented based on "
                          "wlroots, it may not be used in GNOME or similar "
                          "desktop environments");
                    }
                    generalGrimScreenshot(ok, res);
                }
                break;
            }
            default:
                ok = false;
                AbstractLogger::error()
                  << tr("Unable to detect desktop environment (GNOME? KDE? "
                        "Qile? Sway? ...)");
                AbstractLogger::error()
                  << tr("Hint: try setting the XDG_CURRENT_DESKTOP environment "
                        "variable.");
                break;
        }
        if (!ok) {
            AbstractLogger::error() << tr("Unable to capture screen");
        }
        return res;
    }
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) || defined(Q_OS_WIN)
    QRect geometry = desktopGeometry();
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QRect r = primaryScreen->geometry();
    QPixmap desktop =
      primaryScreen->grabWindow(wid,
                                -r.x() / primaryScreen->devicePixelRatio(),
                                -r.y() / primaryScreen->devicePixelRatio(),
                                geometry.width(),
                                geometry.height());

    return selectMonitorAndCrop(desktop, ok);
#endif
}

QRect ScreenGrabber::screenGeometry(QScreen* screen)
{
    QRect geometry;
    if (m_info.waylandDetected()) {
        QPoint topLeft(0, 0);
#ifdef Q_OS_WIN
        for (QScreen* const screen : QGuiApplication::screens()) {
            QPoint topLeftScreen = screen->geometry().topLeft();
            if (topLeft.x() > topLeftScreen.x() ||
                topLeft.y() > topLeftScreen.y()) {
                topLeft = topLeftScreen;
            }
        }
#endif
        geometry = screen->geometry();
        geometry.moveTo(geometry.topLeft() - topLeft);
    } else {
        QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
        geometry = currentScreen->geometry();
    }
    return geometry;
}

QPixmap ScreenGrabber::grabScreen(QScreen* screen, bool& ok)
{
    QPixmap p;
    QRect geometry = screenGeometry(screen);
    if (m_info.waylandDetected()) {
        p = grabEntireDesktop(ok);
        if (ok) {
            return p.copy(geometry);
        }
    } else {
        ok = true;
        return screen->grabWindow(
          0, geometry.x(), geometry.y(), geometry.width(), geometry.height());
    }
    return p;
}

QRect ScreenGrabber::desktopGeometry()
{
    QRect geometry;

    for (QScreen* const screen : QGuiApplication::screens()) {
        QRect scrRect = screen->geometry();
        // Qt6 fix: Don't divide by devicePixelRatio for multi-monitor setups
        // This was causing coordinate offset issues in dual monitor
        // configurations
        // But it still has a screen position in real pixels, not logical ones
        qreal dpr = screen->devicePixelRatio();
        scrRect.moveTo(QPointF(scrRect.x() / dpr, scrRect.y() / dpr).toPoint());
        geometry = geometry.united(scrRect);
    }
    return geometry;
}

QRect ScreenGrabber::logicalDesktopGeometry()
{
    QRect geometry;
    for (QScreen* const screen : QGuiApplication::screens()) {
        QRect scrRect = screen->geometry();
        scrRect.moveTo(scrRect.x(), scrRect.y());
        geometry = geometry.united(scrRect);
    }
    return geometry;
}

void ScreenGrabber::adjustDevicePixelRatio(QPixmap& pixmap)
{
    QRect physicalGeo = desktopGeometry();
    QRect logicalGeo = logicalDesktopGeometry();
    if (pixmap.size() == physicalGeo.size()) {
        // Pixmap is physical size and Qt's DPR is correct
        pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    } else if (pixmap.size() != logicalGeo.size()) {
        // Pixmap is physical size but Qt's DPR is incorrect, calculate actual
        pixmap.setDevicePixelRatio(pixmap.height() * 1.0f /
                                   logicalGeo.height());
    }
}

QScreen* ScreenGrabber::getSelectedScreen() const
{
    if (m_selectedMonitor < 0) {
        return nullptr;
    }
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (m_selectedMonitor >= screens.size()) {
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

    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];

        QPixmap cropped = cropToMonitor(fullScreenshot, i);
        QPixmap thumbnail = cropped.scaled(
          400, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);

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

    // Calculate the scaling factor used in the screenshot
    qreal screenshotScaleX = (qreal)fullScreenshot.width() / totalLogicalWidth;
    qreal screenshotScaleY =
      (qreal)fullScreenshot.height() / totalLogicalHeight;

    qDebug() << tr("Screenshot scale factors: X=%1 Y=%2")
                  .arg(screenshotScaleX)
                  .arg(screenshotScaleY);

    // Calculate crop rect in screenshot coordinates
    // Need to offset by minX/minY to handle negative coordinates
    int cropX = qRound((targetGeometry.x() - minX) * screenshotScaleX);
    int cropY = qRound((targetGeometry.y() - minY) * screenshotScaleY);
    int cropWidth = qRound(targetGeometry.width() * screenshotScaleX);
    int cropHeight = qRound(targetGeometry.height() * screenshotScaleY);

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

    // Set the DPR on the cropped pixmap
    qreal screenshotDpr = (qreal)cropped.width() / targetGeometry.width();
    cropped.setDevicePixelRatio(screenshotDpr);

    return cropped;
}
