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
#include <QScreen>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
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

#include <assert.h>
#include <QCursor>
#include <QPainter>

#ifdef Q_OS_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#else // Q_OS_WINDOWs
#include <Windows.h>
#include <wingdi.h>
#pragma comment (lib, "User32.lib")
#pragma comment (lib, "Gdi32.lib")
#include <QtWinExtras>
#endif

namespace imageutil {

#ifdef Q_OS_LINUX

/* WebCore/plugins/qt/QtX11ImageConversion.cpp */
QImage qimageFromXImage(XImage* xi) {
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    if (xi->depth == 24)
        format = QImage::Format_RGB32;
    else if (xi->depth == 16)
        format = QImage::Format_RGB16;

    QImage image = QImage(reinterpret_cast<uchar*>(xi->data), xi->width, xi->height, xi->bytes_per_line, format).copy();

    // we may have to swap the byte order
    if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && xi->byte_order == MSBFirst)
            || (QSysInfo::ByteOrder == QSysInfo::BigEndian && xi->byte_order == LSBFirst)) {

        for (int i = 0; i < image.height(); i++) {
            if (xi->depth == 16) {
                ushort* p = reinterpret_cast<ushort*>(image.scanLine(i));
                ushort* end = p + image.width();
                while (p < end) {
                    *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                    p++;
                }
            } else {
                uint* p = reinterpret_cast<uint*>(image.scanLine(i));
                uint* end = p + image.width();
                while (p < end) {
                    *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000)
                         | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                    p++;
                }
            }
        }
    }

    // fix-up alpha channel
    if (format == QImage::Format_RGB32) {
        QRgb* p = reinterpret_cast<QRgb*>(image.bits());
        for (int y = 0; y < xi->height; ++y) {
            for (int x = 0; x < xi->width; ++x)
                p[x] |= 0xff000000;
            p += xi->bytes_per_line / 4;
        }
    }

    return image;
}

#endif // Q_OS_LINUX

QPixmap takeScreenShot(const QRect& area) {
    QRect screen; /* interested display area */
    QImage qimage; /* result image */

#ifdef Q_OS_LINUX
    QPoint cursorPos;

    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);

    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);

    const auto goodArea = QRect(0, 0, gwa.width, gwa.height).contains(area);
    if (!goodArea) {
        screen = QRect(0, 0, gwa.width, gwa.height);
        cursorPos = QCursor::pos();
    } else {
        screen = area;
        cursorPos = QCursor::pos() - screen.topLeft();
    }

    XImage* image = XGetImage(display, root, screen.x(), screen.y(), screen.width(), screen.height(), AllPlanes, ZPixmap);
    assert(nullptr != image);

    qimage = qimageFromXImage(image);

    /* draw mouse cursor into QImage
     * https://msnkambule.wordpress.com/2010/04/09/capturing-a-screenshot-showing-mouse-cursor-in-kde/
     * https://github.com/rprichard/x11-canvas-screencast/blob/master/CursorX11.cpp#L31
     * */
    {
        XFixesCursorImage* cursor = XFixesGetCursorImage(display);
        cursorPos -= QPoint(cursor->xhot, cursor->yhot);
        std::vector<uint32_t> pixels(cursor->width * cursor->height);
        for (size_t i = 0; i < pixels.size(); ++i)
            pixels[i] = cursor->pixels[i];
        QImage cursorImage((uchar*)(pixels.data()), cursor->width, cursor->height, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&qimage);
        painter.drawImage(cursorPos, cursorImage);
        XFree(cursor);
    }

    XDestroyImage(image);
    XDestroyWindow(display, root);
    XCloseDisplay(display);

#elif defined(Q_OS_WINDOWS)
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetWindowDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdc);

    RECT rect = { 0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES) };
    const auto goodArea = QRect(rect.left, rect.top, rect.right, rect.bottom).contains(area);
    if (!goodArea) {
        screen = QRect(rect.left, rect.top, rect.right, rect.bottom);
    } else {
        screen = area;
    }

    HBITMAP hbitmap(nullptr);
    hbitmap = CreateCompatibleBitmap(hdc, screen.width(), screen.height());
    SelectObject(hdcMem, hbitmap);
    BitBlt(hdcMem, 0, 0, screen.width(), screen.height(), hdc, screen.x(), screen.y(), SRCCOPY);

    /* draw mouse cursor into DC
     * https://stackoverflow.com/a/48925443/5446734
     * */
    CURSORINFO cursor = { sizeof(cursor) };
    if (GetCursorInfo(&cursor) && cursor.flags == CURSOR_SHOWING) {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        ICONINFO info = { sizeof(info) };
        GetIconInfo(cursor.hCursor, &info);
        const int x = (cursor.ptScreenPos.x - rect.left - rect.left - info.xHotspot) - screen.left();
        const int y = (cursor.ptScreenPos.y - rect.left - rect.left - info.yHotspot) - screen.top();
        BITMAP bmpCursor = { 0 };
        GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
        DrawIconEx(hdcMem, x, y, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight,
                   0, nullptr, DI_NORMAL);
    }

    qimage = QtWin::imageFromHBITMAP(hdc, hbitmap, screen.width(), screen.height());
#endif // Q_OS_LINUX

    return QPixmap::fromImage(qimage);
}

} // namespace imageutil

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
            case DesktopInfo::GNOME: {
                freeDesktopPortal(ok, res);
                break;
            }
            case DesktopInfo::KDE: {
                // https://github.com/KDE/spectacle/blob/517a7baf46a4ca0a45f32fd3f2b1b7210b180134/src/PlatformBackends/KWinWaylandImageGrabber.cpp#L145
                QDBusInterface kwinInterface(
                  QStringLiteral("org.kde.KWin"),
                  QStringLiteral("/Screenshot"),
                  QStringLiteral("org.kde.kwin.Screenshot"));
                QDBusReply<QString> reply =
                  kwinInterface.call(QStringLiteral("screenshotFullscreen"));
                res = QPixmap(reply.value());
                if (!res.isNull()) {
                    QFile dbusResult(reply.value());
                    dbusResult.remove();
                }
                break;
            }
            case DesktopInfo::SWAY: {
                freeDesktopPortal(ok, res);
                break;
            }
            default:
                ok = false;
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
        scrRect.moveTo(scrRect.x() / screen->devicePixelRatio(),
                       scrRect.y() / screen->devicePixelRatio());
        geometry = geometry.united(scrRect);
    }
    return geometry;
}

QPixmap ScreenGrabber::grabEntireDesktopWithCursor(bool & ok)
{
    ok = true;
#if defined(Q_OS_MACOS)
    return grabEntireDesktop(ok);
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_info.waylandDetected()) {
        return grabEntireDesktop(ok);
    }
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX) || defined(Q_OS_WIN)
    const QRect geometry = desktopGeometry();
    QPixmap p = grabEntireDesktop(ok);
    if(!ok)
    {
        return p;
    }
    grabCursor(geometry, p);
    return p;
#endif
}

void ScreenGrabber::grabCursor(const QRect & geometry, QPixmap & res)
{
    QPoint cursorPos;
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);
    const auto goodArea = QRect(0, 0, gwa.width, gwa.height).contains(geometry);
    if (!goodArea) {
        cursorPos = QCursor::pos();
    } else {
        cursorPos = QCursor::pos() - geometry.topLeft();
    }
    XFixesCursorImage* cursor = XFixesGetCursorImage(display);
    cursorPos -= QPoint(cursor->xhot, cursor->yhot);
    std::vector<uint32_t> pixels(cursor->width * cursor->height);
    for (size_t i = 0; i < pixels.size(); ++i)
        pixels[i] = cursor->pixels[i];
    QImage cursorImage((uchar*)(pixels.data()), cursor->width, cursor->height, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&res);
    painter.drawImage(cursorPos, cursorImage);
    XFree(cursor);
    XDestroyWindow(display, root);
    XCloseDisplay(display);
}
