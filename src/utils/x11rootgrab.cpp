// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#include "x11rootgrab.h"

#include <QImage>

#if defined(FLAMESHOT_HAVE_X11)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

QPixmap x11RootScreenshot()
{
#if defined(FLAMESHOT_HAVE_X11)
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return QPixmap();
    }

    const Window root = DefaultRootWindow(display);
    XWindowAttributes attributes;
    if (!XGetWindowAttributes(display, root, &attributes)) {
        XCloseDisplay(display);
        return QPixmap();
    }

    XImage* image = XGetImage(display,
                              root,
                              0,
                              0,
                              attributes.width,
                              attributes.height,
                              AllPlanes,
                              ZPixmap);
    if (!image) {
        XCloseDisplay(display);
        return QPixmap();
    }

    QImage result(image->width, image->height, QImage::Format_RGB32);
    for (int y = 0; y < image->height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < image->width; ++x) {
            const unsigned long pixel = XGetPixel(image, x, y);
            line[x] = qRgb((pixel >> 16) & 0xff,
                           (pixel >> 8) & 0xff,
                           pixel & 0xff);
        }
    }

    XDestroyImage(image);
    XCloseDisplay(display);
    return QPixmap::fromImage(result);
#else
    return QPixmap();
#endif
}

QRect x11RootGeometry()
{
#if defined(FLAMESHOT_HAVE_X11)
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        return {};
    }

    const Window root = DefaultRootWindow(display);
    XWindowAttributes attributes;
    QRect rect;
    if (XGetWindowAttributes(display, root, &attributes)) {
        rect = QRect(0, 0, attributes.width, attributes.height);
    }
    XCloseDisplay(display);
    return rect;
#else
    return {};
#endif
}
