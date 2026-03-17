#pragma once

#include <QImage>
#include <QRect>

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#endif

struct CaptureRequest
{
#ifdef Q_OS_LINUX
    Display* display = nullptr;
    ::Window target = 0;
#endif
    QRect rect;
    bool targetIsRoot = true;
};

class ScreenCaptureBackend
{
public:
    static bool isWaylandSession();
    static bool isX11Session();

    static QImage grab(const CaptureRequest& request);

private:
#ifdef Q_OS_LINUX
    static QImage grabWithXGetImage(const CaptureRequest& request);
#endif
};
