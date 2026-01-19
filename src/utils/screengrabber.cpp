// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "screengrabber.h"
#include "abstractlogger.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QApplication>
#include <QGuiApplication>
#include <QPixmap>
#include <QProcess>
#include <QScreen>

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
    const auto gotSignal = [&res, &loop, this](uint status,
                                               const QVariantMap& map) {
        if (status == 0) {
            // Parse this as URI to handle unicode properly
            QUrl uri = map.value("uri").toString();
            QString uriString = uri.toLocalFile();
            res = QPixmap(uriString);
            adjustDevicePixelRatio(res);
            QFile imgFile(uriString);
            imgFile.remove();
        }
        loop.quit();
    };

    // prevent racy situations and listen before calling screenshot
    QMetaObject::Connection conn = QObject::connect(
      request, &org::freedesktop::portal::Request::Response, gotSignal);

    screenshotInterface.call(
      QStringLiteral("Screenshot"),
      "",
      QMap<QString, QVariant>({ { "handle_token", QVariant(token) },
                                { "interactive", QVariant(false) } }));

    loop.exec();
    QObject::disconnect(conn);
    request->Close().waitForFinished();
    request->deleteLater();

    if (res.isNull()) {
        ok = false;
    }
#endif
}

QPixmap ScreenGrabber::grabEntireDesktop(bool& ok)
{
    ok = true;
    int wid = 0;

#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    QPixmap screenPixmap(
      currentScreen->grabWindow(wid,
                                currentScreen->geometry().x(),
                                currentScreen->geometry().y(),
                                currentScreen->geometry().width(),
                                currentScreen->geometry().height()));
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

    // Qt6 fix: Create a composite image from all screens to handle
    // multi-monitor setups where screens have different positions/heights.
    // This fixes the dual monitor offset bug and handles edge cases where
    // the desktop bounding box includes virtual space.
    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QRect r = primaryScreen->geometry();
    QPixmap desktop(geometry.size());
    desktop.fill(Qt::black); // Fill with black background
    desktop =
      primaryScreen->grabWindow(wid,
                                -r.x() / primaryScreen->devicePixelRatio(),
                                -r.y() / primaryScreen->devicePixelRatio(),
                                geometry.width(),
                                geometry.height());
    return desktop;
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
