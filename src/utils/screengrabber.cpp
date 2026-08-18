
#include "screengrabber.h"
#include "core/qguiappcurrentscreen.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"
#include "utils/monitorpickersurface.h"
#include "utils/monitorpreview.h"
#include "utils/systemnotification.h"

#include <QApplication>
#include <QEventLoop>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QScreen>
#include <QStringList>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QtAlgorithms>
#include <algorithm>
#include <functional>

#ifdef FLAMESHOT_DEBUG_CAPTURE
#include <QDebug>
#endif

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
#include "request.h"
#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDir>
#include <QUrl>
#include <QUuid>
#endif

bool ScreenGrabber::m_monitorSelectionActive = false;

QPixmap& ScreenGrabber::fullScreenshotStorage()
{
    static QPixmap shot;
    return shot;
}

ScreenGrabber::ScreenGrabber(QObject* parent)
  : QObject(parent)
  , m_selectedMonitor(-1)
  , m_highlightedMonitorPreview(-1)
  , m_monitorSelectionLoop(nullptr)
  , m_userCancelled(false)
{
    // Increase image allocation limit for large screenshots
    // (multi-monitor/high-DPI) Default is 128MB, set to 1GB to handle 4K+
    // multi-monitor setups
    QImageReader::setAllocationLimit(1024);
}

ScreenGrabber::PortalStatus ScreenGrabber::freeDesktopPortal(
  QPixmap& res,
  QString& errorDetail)
{

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    auto* connectionInterface = QDBusConnection::sessionBus().interface();
    auto service = QStringLiteral("org.freedesktop.portal.Desktop");

    if (!connectionInterface->isServiceRegistered(service)) {
        errorDetail =
          tr("Could not locate the `org.freedesktop.portal.Desktop` service");
        return PortalStatus::Unavailable;
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

    bool timedOut = false;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(15000); // 15 second timeout

    QObject::connect(&timeout, &QTimer::timeout, &loop, [&loop, &timedOut]() {
        timedOut = true;
        loop.quit();
    });
    timeout.start();

    // Build a non-empty parent_window handle. xdg-desktop-portal-gnome
    // (>= 46) rejects an empty string with "Failed to associate portal
    // window with parent window ''" and the Screenshot request fails.
    // On X11 we pass a real x11:<hex> handle from an offscreen QWidget.
    // On Wayland we fall back to an empty string inside a wayland: prefix
    // (xdg-desktop-portal-gnome treats unknown handles as no-parent and
    // proceeds, instead of rejecting outright).
    QString parentWindow;
    QWidget parentDummy;
    parentDummy.setAttribute(Qt::WA_DontShowOnScreen, true);
    parentDummy.resize(1, 1);
    parentDummy.show();
    if (QGuiApplication::platformName() == QLatin1String("wayland")) {
        parentWindow = QStringLiteral("wayland:");
    } else {
        parentWindow =
          QStringLiteral("x11:0x%1").arg(parentDummy.winId(), 0, 16);
    }

    QDBusMessage reply = screenshotInterface.call(
      QStringLiteral("Screenshot"),
      parentWindow,
      QMap<QString, QVariant>({ { "handle_token", QVariant(token) },
                                { "interactive", QVariant(false) } }));

    if (reply.type() == QDBusMessage::ErrorMessage) {
        // No backend provides org.freedesktop.portal.Screenshot (or the
        // portal rejected the request outright); the Response signal will
        // never arrive, so fail now instead of waiting for the timeout.
        QObject::disconnect(conn);
        request->deleteLater();
        errorDetail =
          tr("The `org.freedesktop.portal.Screenshot` request failed: %1")
            .arg(reply.errorMessage());
        return PortalStatus::Unavailable;
    }

    loop.exec();
    timeout.stop();
    QObject::disconnect(conn);
    request->Close().waitForFinished();
    request->deleteLater();

    if (timedOut) {
        errorDetail =
          tr("The xdg-desktop-portal backend did not respond "
             "If you are on wayland make sure an xdg-desktop-portal backend "
             "for your desktop is "
             "installed and properly configured.\n \n"
             "If on X11 enable Legacy X11 method in the General Settings");
        return PortalStatus::Failed;
    }

    if (res.isNull()) {
        return PortalStatus::Failed;
    }

#ifdef FLAMESHOT_DEBUG_CAPTURE
    qDebug() << tr("FreeDesktop portal screenshot size: %1x%2, DPR: %3")
                  .arg(res.width())
                  .arg(res.height())
                  .arg(res.devicePixelRatio());
#endif
    return PortalStatus::Success;
#else
    Q_UNUSED(res)
    Q_UNUSED(errorDetail)
    return PortalStatus::Failed;
#endif
}

QPixmap ScreenGrabber::unixScreenshot(bool& ok)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QPixmap screenshot;

    if (!m_info.waylandDetected() && ConfigHandler().useX11LegacyScreenshot()) {
        screenshot = x11LegacyScreenshot();
        ok = !screenshot.isNull();
        if (!ok) {
            AbstractLogger::error() << tr("Unable to capture screen");
        }
        return screenshot;
    }

    QString portalError;
    const PortalStatus status = freeDesktopPortal(screenshot, portalError);
    ok = status == PortalStatus::Success;

    if (status == PortalStatus::Unavailable && !m_info.waylandDetected()) {
        // The portal cannot take screenshots here, which is common on X11
        // window managers such as i3 or XMonad, so take the native X11
        // grab instead.
        AbstractLogger::info(AbstractLogger::Stderr | AbstractLogger::LogFile)
          << tr("Screenshot portal unavailable, using direct X11 capture");
        screenshot = x11LegacyScreenshot();
        ok = !screenshot.isNull();
    }

    if (!ok) {
        if (!portalError.isEmpty()) {
            AbstractLogger::error() << portalError;
        }
        AbstractLogger::error() << tr("Unable to capture screen");
    }

    return screenshot;
#else
    ok = false;
    return QPixmap();
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

    // Skip the selection dialog and capture the monitor under the cursor
    // directly (on X11 the cursor position is reliable, on Wayland it is
    // recovered from the compositor's placement of a probe window).
    QScreen* cursorScreen = cursorMonitor();
    if (cursorScreen) {
        const int monitorIndex = screens.indexOf(cursorScreen);
        if (monitorIndex >= 0) {
            m_selectedMonitor = monitorIndex;
            return cropToMonitor(fullScreenshot, monitorIndex);
        }
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
    const QList<QWidget*> containers =
      createMonitorPreviews(fullScreenshot, cursorScreen);

    // Wait for user to select a monitor
    QEventLoop loop;
    m_monitorSelectionLoop = &loop;
    loop.exec();
    m_monitorSelectionLoop = nullptr;

    qDeleteAll(containers);
    m_monitorPreviews.clear();
    m_highlightedMonitorPreview = -1;
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
    m_selectedMonitor = QGuiApplication::screens().indexOf(currentScreen);
    if (m_selectedMonitor < 0) {
        AbstractLogger::error() << tr("Unable to get current screen");
        ok = false;
        return QPixmap();
    }
    const QRect geom = currentScreen->geometry();
    screenshot = currentScreen->grabWindow(
      wid, geom.x(), geom.y(), geom.width(), geom.height());
    screenshot.setDevicePixelRatio(currentScreen->devicePixelRatio());
    return screenshot;

#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    screenshot = unixScreenshot(ok);
    if (!ok) {
        return QPixmap();
    }
    // Keep the full desktop around so the capture UI can build the
    // per-monitor previews of the monitors that are not being captured.
    fullScreenshotStorage() = screenshot;
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

QPixmap ScreenGrabber::grabFullDesktop(bool& ok)
{
    ok = true;
    QPixmap screenshot;

#if defined(Q_OS_MACOS)
    // On macOS, composite all screens into a single pixmap.
    const QList<QScreen*> screens = QGuiApplication::screens();
    QRect totalGeom;
    for (QScreen* s : screens) {
        totalGeom = totalGeom.united(s->geometry());
    }
    qreal maxDpr = 1.0;
    for (QScreen* s : screens) {
        maxDpr = qMax(maxDpr, s->devicePixelRatio());
    }
    screenshot = QPixmap(qRound(totalGeom.width() * maxDpr),
                         qRound(totalGeom.height() * maxDpr));
    screenshot.setDevicePixelRatio(maxDpr);
    screenshot.fill(Qt::black);
    QPainter painter(&screenshot);
    for (QScreen* s : screens) {
        QRect geom = s->geometry();
        QPixmap p = s->grabWindow(0);
        QPoint offset = geom.topLeft() - totalGeom.topLeft();
        painter.drawPixmap(offset, p);
    }
    painter.end();
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    screenshot = unixScreenshot(ok);
#elif defined(Q_OS_WIN)
    screenshot = windowsScreenshot(0);
#endif

    return screenshot;
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
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    const QList<QScreen*> screens = QGuiApplication::screens();
    int screenIndex = screens.indexOf(screen);

    p = grabEntireDesktop(ok, screenIndex);
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

namespace {

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))

// Quits the cursor-probe event loop as soon as the watched widget is
// exposed. On Wayland the compositor assigns the output before mapping the
// window, so by the time the Expose event arrives the window's screen is
// final; waiting for QWindow::screenChanged alone costs the full timeout
// whenever the compositor places the window on the screen Qt already
// assigned it (e.g. the pointer is on the primary monitor).
class ProbeExposeFilter : public QObject
{
public:
    explicit ProbeExposeFilter(QObject* parent = nullptr)
      : QObject(parent)
    {}

    std::function<void()> onExpose;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::Expose && onExpose) {
            onExpose();
        }
        return QObject::eventFilter(watched, event);
    }
};

// Skip a `a{sv}` (string -> variant) dictionary on the QDBusArgument cursor.
void skipVariantMap(const QDBusArgument& arg)
{
    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg >> key >> value;
        Q_UNUSED(key);
        Q_UNUSED(value);
    }
    arg.endMap();
}

// Skip a single monitor mode struct: `(s i i d d ad a{sv})`.
void skipMonitorMode(const QDBusArgument& arg)
{
    arg.beginStructure();
    QString id;
    int width, height;
    double refreshRate, scale;
    arg >> id >> width >> height >> refreshRate >> scale;
    Q_UNUSED(id);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(refreshRate);
    Q_UNUSED(scale);
    arg.beginArray();
    while (!arg.atEnd()) {
        double s;
        arg >> s;
        Q_UNUSED(s);
    }
    arg.endArray();
    skipVariantMap(arg);
    arg.endStructure();
}

// Skip the monitors array `a((ssss)a((siiddada{sv}))a{sv})`; the primary
// monitor is reported by the logical monitors array instead.
void skipMonitorsArray(const QDBusArgument& arg)
{
    arg.beginArray();
    while (!arg.atEnd()) {
        arg.beginStructure();
        QString connector, vendor, product, serial;
        arg.beginStructure();
        arg >> connector >> vendor >> product >> serial;
        Q_UNUSED(connector);
        Q_UNUSED(vendor);
        Q_UNUSED(product);
        Q_UNUSED(serial);
        arg.endStructure();
        arg.beginArray();
        while (!arg.atEnd()) {
            skipMonitorMode(arg);
        }
        arg.endArray();
        skipVariantMap(arg);
        arg.endStructure();
    }
    arg.endArray();
}

#endif

} // namespace

QScreen* ScreenGrabber::reliablePrimaryScreen()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    DesktopInfo info;
    if (info.waylandDetected() && info.windowManager() == DesktopInfo::GNOME) {
        // Qt's wayland plugin does not implement the wp_primary_output
        // protocol, so QGuiApplication::primaryScreen() on GNOME Wayland
        // returns the first wl_output advertised by the compositor (usually
        // the built-in panel) instead of the monitor the user actually
        // configured as primary. Ask mutter directly; it is the authoritative
        // source for the primary monitor on GNOME.
        // GetCurrentState returns a struct whose fields QDBusMessage exposes
        // as separate arguments:
        //   u, a((ssss)a((siiddada{sv}))a{sv}), a(iiduba(ssss)a{sv}), a{sv}
        QDBusInterface displayConfig(
          QStringLiteral("org.gnome.Mutter.DisplayConfig"),
          QStringLiteral("/org/gnome/Mutter/DisplayConfig"),
          QStringLiteral("org.gnome.Mutter.DisplayConfig"));
        QDBusMessage msg =
          displayConfig.call(QStringLiteral("GetCurrentState"));
        if (msg.type() == QDBusMessage::ReplyMessage) {
            const QList<QVariant>& replyArgs = msg.arguments();
            if (replyArgs.size() >= 3 &&
                replyArgs.at(1).canConvert<QDBusArgument>() &&
                replyArgs.at(2).canConvert<QDBusArgument>()) {
                const QDBusArgument monitorsArg =
                  replyArgs.at(1).value<QDBusArgument>();
                skipMonitorsArray(monitorsArg);

                // Find the primary logical monitor and the connector(s)
                // mapped to it.
                QString primaryConnector;
                const QDBusArgument logicalArg =
                  replyArgs.at(2).value<QDBusArgument>();
                logicalArg.beginArray();
                while (!logicalArg.atEnd() && primaryConnector.isEmpty()) {
                    logicalArg.beginStructure();
                    int x, y;
                    double scale;
                    uint transform;
                    bool primary;
                    logicalArg >> x >> y >> scale >> transform >> primary;
                    Q_UNUSED(x);
                    Q_UNUSED(y);
                    Q_UNUSED(scale);
                    Q_UNUSED(transform);
                    QStringList connectors;
                    logicalArg.beginArray();
                    while (!logicalArg.atEnd()) {
                        QString connector, vendor, product, serial;
                        logicalArg.beginStructure();
                        logicalArg >> connector >> vendor >> product >> serial;
                        Q_UNUSED(connector);
                        Q_UNUSED(vendor);
                        Q_UNUSED(product);
                        Q_UNUSED(serial);
                        logicalArg.endStructure();
                        connectors.append(connector);
                    }
                    logicalArg.endArray();
                    skipVariantMap(logicalArg);
                    logicalArg.endStructure();
                    if (primary && !connectors.isEmpty()) {
                        primaryConnector = connectors.first();
                    }
                }
                logicalArg.endArray();

                if (!primaryConnector.isEmpty()) {
                    const QList<QScreen*> screens = QGuiApplication::screens();
                    for (QScreen* screen : screens) {
                        if (screen->name() == primaryConnector) {
                            return screen;
                        }
                    }
                }
            }
        }
    }
#endif
    return QGuiApplication::primaryScreen();
}

QScreen* ScreenGrabber::cursorMonitor()
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    if (!m_info.waylandDetected()) {
        // On X11 the global cursor position is reliable.
        return QGuiAppCurrentScreen().currentScreen();
    }

    // On Wayland the compositor only reports the pointer position to clients
    // whose surface is under the pointer, so it cannot be queried before any
    // window is mapped. However, compositors place new windows on the monitor
    // containing the pointer (mutter: meta_backend_get_current_logical_monitor
    // when the client does not request a position), so mapping a tiny
    // invisible probe window reveals the pointer's monitor through the screen
    // the compositor assigns to it.
    QWidget probe(nullptr,
                  Qt::Window | Qt::FramelessWindowHint |
                    Qt::WindowStaysOnTopHint | Qt::Tool);
    probe.setAttribute(Qt::WA_TranslucentBackground);
    probe.setAttribute(Qt::WA_ShowWithoutActivating);
    probe.setAttribute(Qt::WA_TransparentForMouseEvents);
    probe.resize(1, 1);

    QScreen* result = nullptr;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    // Backstop only: the loop normally exits when the probe window is
    // exposed (or its screen changes), both of which happen within a frame
    // or two of show(), so this is never reached on a healthy session.
    timeout.setInterval(400);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Quit as soon as the probe is exposed: on Wayland the compositor
    // assigns the output before mapping the window, so its screen is final
    // by the time the Expose event arrives. Relying on screenChanged alone
    // made PrtSc feel unresponsive (~1s) whenever the compositor placed the
    // window on the screen Qt had already assigned it (e.g. the pointer is
    // on the primary monitor): no screenChanged is emitted in that case and
    // the loop waited out the full timeout.
    ProbeExposeFilter exposeFilter(&probe);
    exposeFilter.onExpose = [&loop]() {
        // Defer the quit by one event-loop iteration so any screenChanged
        // already queued for the same compositor round-trip is delivered
        // first.
        QTimer::singleShot(0, &loop, &QEventLoop::quit);
    };
    probe.installEventFilter(&exposeFilter);

    probe.show();
    if (QWindow* handle = probe.windowHandle()) {
        QObject::connect(handle,
                         &QWindow::screenChanged,
                         &loop,
                         [&result, &loop](QScreen* screen) {
                             if (screen) {
                                 result = screen;
                                 loop.quit();
                             }
                         });
    }
    timeout.start();
    loop.exec();
    QScreen* probeScreen =
      probe.windowHandle() ? probe.windowHandle()->screen() : nullptr;
    probe.close();

    // If the compositor placed the window on its initial screen, no
    // screenChanged was emitted; the current screen is still the answer.
    if (!result && probeScreen) {
        result = probeScreen;
    }
    return result;
#else
    return nullptr;
#endif
}

QScreen* ScreenGrabber::getSelectedScreen() const
{
    const QList<QScreen*> screens = QGuiApplication::screens();

    if ((m_selectedMonitor < 0) || (m_selectedMonitor >= screens.size())) {
        return nullptr;
    }

    return screens[m_selectedMonitor];
}

QList<QWidget*> ScreenGrabber::createMonitorPreviews(
  const QPixmap& fullScreenshot,
  QScreen* cursorScreen)
{
    const QList<QScreen*> screens = QGuiApplication::screens();
    m_monitorPreviews.clear();
    m_highlightedMonitorPreview = -1;

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

    // A compact picker is shown at the bottom of every monitor.
    //
    // On Wayland the compositor ignores move()/setScreen() for regular
    // windows (they are placed on the monitor containing the pointer), so each
    // picker is shown FULLSCREEN -- the only state where the requested output
    // is honored. Fullscreen windows are composited as opaque (transparency
    // comes out black), so the picker paints the monitor's frozen desktop as
    // its background and the compact strip sits on top of it.
    //
    // On X11 a small translucent window is moved to the bottom-center instead
    // and clicks above the strip pass through to the desktop.
    QList<QWidget*> pickers;
    const bool wayland = m_info.waylandDetected();

    for (int screenIndex = 0; screenIndex < screens.size(); ++screenIndex) {
        QScreen* pickerScreen = screens[screenIndex];
        const QPixmap monitorShot = cropToMonitor(fullScreenshot, screenIndex);

        QWidget* picker =
          wayland
            ? new MonitorPickerSurface(monitorShot,
                                       nullptr,
                                       Qt::Window | Qt::FramelessWindowHint |
                                         Qt::WindowStaysOnTopHint)
            : new QWidget(nullptr,
                          Qt::Window | Qt::FramelessWindowHint |
                            Qt::WindowStaysOnTopHint);
        if (!wayland) {
            picker->setAttribute(Qt::WA_TranslucentBackground);
            picker->setStyleSheet("QWidget { background-color: transparent; }");
        }
        // Remember which monitor this picker represents so a click on its
        // background selects that monitor (Wayland fullscreen pickers receive
        // all clicks; there is no input pass-through without a mask).
        picker->setProperty("monitorIndex", screenIndex);
        picker->installEventFilter(this); // For ESC / background click
        picker->setFocusPolicy(Qt::StrongFocus);

        // Content is pinned to the bottom of the window.
        QVBoxLayout* outerLayout = new QVBoxLayout(picker);
        outerLayout->setContentsMargins(0, 0, 0, 12);
        outerLayout->addStretch(1);

        QWidget* rowWidget = new QWidget(picker);
        rowWidget->setAttribute(Qt::WA_TranslucentBackground);
        rowWidget->setStyleSheet("QWidget { background-color: transparent; }");
        QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setSpacing(10);
        rowLayout->setContentsMargins(10, 0, 10, 0);

        for (int i = 0; i < screens.size(); ++i) {
            QPixmap cropped = cropToMonitor(fullScreenshot, i);
            QPixmap thumbnail = cropped.scaled(
              240, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            thumbnail.setDevicePixelRatio(1.0);

            MonitorPreview* preview = new MonitorPreview(
              i, screens[i], thumbnail, rowWidget, /*compact=*/true);

            connect(preview,
                    &MonitorPreview::monitorSelected,
                    this,
                    [this](int index) { selectMonitor(index); });

            m_monitorPreviews.append(preview);
            rowLayout->addWidget(preview);
        }
        outerLayout->addWidget(rowWidget, 0, Qt::AlignHCenter);

        // Associate the native window with the target screen first so that on
        // Wayland the compositor maps it on the right output (winId() forces
        // the native window to exist; QWindow::setScreen() recreates it there).
        picker->winId();
        if (QWindow* handle = picker->windowHandle()) {
            handle->setScreen(pickerScreen);
        }

        const QRect screenGeom = pickerScreen->geometry();
        if (wayland) {
            picker->resize(screenGeom.size());
            picker->layout()->activate();
        } else {
            picker->adjustSize();
            picker->move(
              screenGeom.x() + (screenGeom.width() - picker->width()) / 2,
              screenGeom.y() + screenGeom.height() - picker->height() - 16);
        }

        pickers.append(picker);
    }

    // Highlight the monitor the pointer is on (the selection is skipped when
    // it is known); fall back to the real primary monitor.
    QScreen* initialScreen =
      cursorScreen ? cursorScreen : reliablePrimaryScreen();
    const int initialMonitorIndex =
      initialScreen ? screens.indexOf(initialScreen) : -1;
    setHighlightedMonitorPreview(initialMonitorIndex >= 0 ? initialMonitorIndex
                                                          : 0);

    for (QWidget* picker : pickers) {
        if (wayland) {
            picker->showFullScreen();
        } else {
            picker->show();
        }
        picker->raise();
        picker->activateWindow();
        picker->setFocus(Qt::ActiveWindowFocusReason);
    }
    return pickers;
}

void ScreenGrabber::cancelMonitorSelection()
{
    m_selectedMonitor = -1;
    m_userCancelled = true;
    if (m_monitorSelectionLoop) {
        m_monitorSelectionLoop->quit();
    }
}

void ScreenGrabber::selectMonitor(int monitorIndex)
{
    m_selectedMonitor = monitorIndex;
    if (m_monitorSelectionLoop) {
        m_monitorSelectionLoop->quit();
    }
}

void ScreenGrabber::setHighlightedMonitorPreview(int monitorIndex)
{
    if (m_monitorPreviews.isEmpty()) {
        m_highlightedMonitorPreview = -1;
        return;
    }

    const int monitorCount = QGuiApplication::screens().size();
    int normalizedIndex = monitorIndex % monitorCount;
    if (normalizedIndex < 0) {
        normalizedIndex += monitorCount;
    }

    // Every picker window shows one preview per monitor, so highlight the
    // matching preview in all of them.
    for (MonitorPreview* preview : m_monitorPreviews) {
        preview->setSelected(preview->monitorIndex() == normalizedIndex);
    }
    m_highlightedMonitorPreview = normalizedIndex;
}

bool ScreenGrabber::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Close) {
        cancelMonitorSelection();
        return true;
    }
    // A click on a picker's background selects the monitor that picker
    // represents. This is how the Wayland compact pickers are operated: they
    // are fullscreen windows (the only way to pin them to a monitor) and
    // receive every click, since there is no input pass-through without a
    // mask (masks render the unmasked area black on Wayland).
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            const int monitorIndex = obj->property("monitorIndex").toInt();
            if (obj->property("monitorIndex").isValid()) {
                selectMonitor(monitorIndex);
                return true;
            }
        }
    }
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            cancelMonitorSelection();
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
    int minX = INT_MAX, minY = INT_MAX;
    int maxX = INT_MIN, maxY = INT_MIN;

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

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
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

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
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

QPixmap ScreenGrabber::x11LegacyScreenshot()
{
    const QList<QScreen*> screens = QGuiApplication::screens();

    if (screens.isEmpty()) {
        return QPixmap();
    }

    if (screens.size() == 1) {
        QScreen* screen = screens.first();
        QPixmap p = screen->grabWindow(0);
        p.setDevicePixelRatio(screen->devicePixelRatio());
        return p;
    }

    // Composite all screens using logical geometry.
    // On i3 (tested) DPR is uniform so we don't need the per-screen
    // physical pixel math that the Windows backend does. Not sure if this is
    // true for other DE's like xmonad.
    QRect totalGeom;
    for (QScreen* s : screens) {
        totalGeom = totalGeom.united(s->geometry());
    }

    qreal dpr = screens.first()->devicePixelRatio();
    QPixmap desktop(qRound(totalGeom.width() * dpr),
                    qRound(totalGeom.height() * dpr));
    desktop.setDevicePixelRatio(dpr);
    desktop.fill(Qt::black);

    QPainter painter(&desktop);
    for (QScreen* s : screens) {
        QPixmap p = s->grabWindow(0);
        QPoint offset = s->geometry().topLeft() - totalGeom.topLeft();
        painter.drawPixmap(offset, p);
    }
    painter.end();

    return desktop;
}
