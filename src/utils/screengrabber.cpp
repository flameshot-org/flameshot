// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "screengrabber.h"
#include "abstractlogger.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QPixmap>
#include <QProcess>
#include <QScreen>
#include <QFile>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include "request.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QUrl>
#include <QUuid>
#include <unistd.h>

static QImage allocateImage(const QVariantMap &metadata)
{
    bool ok;

    const uint width = metadata.value(QStringLiteral("width")).toUInt(&ok);
    if (!ok) {
        return QImage();
    }

    const uint height = metadata.value(QStringLiteral("height")).toUInt(&ok);
    if (!ok) {
        return QImage();
    }

    const uint format = metadata.value(QStringLiteral("format")).toUInt(&ok);
    if (!ok) {
        return QImage();
    }

    return QImage(width, height, QImage::Format(format));
}

static QPixmap readImage(int fileDescriptor, const QVariantMap &metadata)
{
    QFile file;
    if (!file.open(fileDescriptor, QFileDevice::ReadOnly, QFileDevice::AutoCloseHandle)) {
        close(fileDescriptor);
        return QPixmap();
    }

    QImage result = allocateImage(metadata);
    if (result.isNull()) {
        return QPixmap();
    }

    QDataStream stream(&file);
    stream.readRawData(reinterpret_cast<char *>(result.bits()), result.sizeInBytes());
    return QPixmap::fromImage(result);
}

#endif

ScreenGrabber::ScreenGrabber(QObject* parent)
  : QObject(parent)
{}


void ScreenGrabber::ScreenShot2Portal(bool& ok, QPixmap& res)
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    static const QString s_screenShotService = QStringLiteral("org.kde.KWin.ScreenShot2");
    static const QString s_screenShotObjectPath = QStringLiteral("/org/kde/KWin/ScreenShot2");
    static const QString s_screenShotInterface = QStringLiteral("org.kde.KWin.ScreenShot2");


    // Do not set the O_NONBLOCK flag. Code that reads data from the pipe assumes
    // that read() will block if there is no any data yet.
    int pipeFds[2];
    pipe(pipeFds);

    QDBusMessage message = QDBusMessage::createMethodCall(s_screenShotService, s_screenShotObjectPath, s_screenShotInterface, "CaptureScreen");
    QVariantMap options;

    options.insert(QStringLiteral("native-resolution"), true);
    options.insert(QStringLiteral("include-decoration"), true);
    options.insert(QStringLiteral("include-cursor"), true);

    QVariantList dbusArguments;
    dbusArguments << QString(qApp->primaryScreen()->name()) << options;

    dbusArguments.append(QVariant::fromValue(QDBusUnixFileDescriptor(pipeFds[1])));
    message.setArguments(dbusArguments);

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    close(pipeFds[1]);

    QEventLoop loop;

    auto watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [&]() {
        watcher->deleteLater();
        const QDBusPendingReply<QVariantMap> reply = *watcher;

        if (!reply.isError())
        {
            ok = true;
            res = readImage(pipeFds[0], reply);
            res.setDevicePixelRatio(qApp->devicePixelRatio());
        }
        else {
            qWarning() << "Screenshot request failed:" << reply.error().message();
        }
        loop.quit();
    });

    loop.exec();

    if (res.isNull()) {
        ok = false;
    }
#endif
}

void ScreenGrabber::generalGrimScreenshot(bool& ok, QPixmap& res)
{
#ifdef USE_WAYLAND_GRIM
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QProcess Process;
    QString program = "grim";
    QStringList arguments;
    arguments << "-";
    Process.start(program, arguments);
    if (Process.waitForFinished()) {
        res.loadFromData(Process.readAll());
        ok = true;
    } else {
        ok = false;
        AbstractLogger::error()
          << tr("The universal wayland screen capture adapter requires Grim as "
                "the screen capture component of wayland. If the screen "
                "capture component is missing, please install it!");
    }
#endif
#endif
}

void ScreenGrabber::freeDesktopPortal(bool& ok, QPixmap& res)
{

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QDBusInterface screenshotInterface(
      QStringLiteral("org.freedesktop.portal.Desktop"),
      QStringLiteral("/org/freedesktop/portal/desktop"),
      QStringLiteral("org.freedesktop.portal.Screenshot"));

    // unique token
    QString token =
      QUuid::createUuid().toString().remove('-').remove('{').remove('}');

    // premake interface
    auto* request = new OrgFreedesktopPortalRequestInterface(
      QStringLiteral("org.freedesktop.portal.Desktop"),
      "/org/freedesktop/portal/desktop/request/" +
        QDBusConnection::sessionBus().baseService().remove(':').replace('.',
                                                                        '_') +
        "/" + token,
      QDBusConnection::sessionBus(),
      this);

    QEventLoop loop;
    const auto gotSignal = [&res, &loop](uint status, const QVariantMap& map) {
        if (status == 0) {
            // Parse this as URI to handle unicode properly
            QUrl uri = map.value("uri").toString();
            QString uriString = uri.toLocalFile();
            res = QPixmap(uriString);
            res.setDevicePixelRatio(qApp->devicePixelRatio());
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
#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    QPixmap screenPixmap(
      currentScreen->grabWindow(QApplication::desktop()->winId(),
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
                freeDesktopPortal(ok, res);
            case DesktopInfo::KDE:
                ScreenShot2Portal(ok, res);
                break;
            case DesktopInfo::QTILE:
            case DesktopInfo::SWAY:
            case DesktopInfo::HYPRLAND:
            case DesktopInfo::OTHER: {
#ifndef USE_WAYLAND_GRIM
                AbstractLogger::warning() << tr(
                  "If the USE_WAYLAND_GRIM option is not activated, the dbus "
                  "protocol will be used. It should be noted that using the "
                  "dbus protocol under wayland is not recommended. It is "
                  "recommended to recompile with the USE_WAYLAND_GRIM flag to "
                  "activate the grim-based general wayland screenshot adapter");
                freeDesktopPortal(ok, res);
#else
                AbstractLogger::warning()
                  << tr("grim's screenshot component is implemented based on "
                        "wlroots, it may not be used in GNOME or similar "
                        "desktop environments");
                generalGrimScreenshot(ok, res);
#endif
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
    QPixmap p(QApplication::primaryScreen()->grabWindow(
      QApplication::desktop()->winId(),
      geometry.x(),
      geometry.y(),
      geometry.width(),
      geometry.height()));
    auto screenNumber = QApplication::desktop()->screenNumber();
    QScreen* screen = QApplication::screens()[screenNumber];
    p.setDevicePixelRatio(screen->devicePixelRatio());
    return p;
#endif
}

QRect ScreenGrabber::screenGeometry(QScreen* screen)
{
    QPixmap p;
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
        return screen->grabWindow(QApplication::desktop()->winId(),
                                  geometry.x(),
                                  geometry.y(),
                                  geometry.width(),
                                  geometry.height());
    }
    return p;
}

QRect ScreenGrabber::desktopGeometry()
{
    QRect geometry;

    for (QScreen* const screen : QGuiApplication::screens()) {
        QRect scrRect = screen->geometry();
        scrRect.moveTo(scrRect.x() / screen->devicePixelRatio(),
                       scrRect.y() / screen->devicePixelRatio());
        geometry = geometry.united(scrRect);
    }
    return geometry;
}
