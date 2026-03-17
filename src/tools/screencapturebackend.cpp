#include <QDebug>
#include "screencapturebackend.h"

#include <QByteArray>
#include <QtAlgorithms>

#ifdef Q_OS_LINUX
#include <X11/Xutil.h>

namespace {

static int g_x11LastErrorCode = 0;
static int g_x11LastRequestCode = 0;
static int g_x11LastMinorCode = 0;

int temporaryX11ErrorHandler(Display* /*display*/, XErrorEvent* event)
{
    if (!event) {
        return 0;
    }

    g_x11LastErrorCode = event->error_code;
    g_x11LastRequestCode = event->request_code;
    g_x11LastMinorCode = event->minor_code;
    return 0;
}

class ScopedX11ErrorTrap
{
public:
    explicit ScopedX11ErrorTrap(Display* dpy)
      : m_display(dpy)
    {
        g_x11LastErrorCode = 0;
        g_x11LastRequestCode = 0;
        g_x11LastMinorCode = 0;

        if (m_display) {
            XSync(m_display, False);
            m_previousHandler = XSetErrorHandler(temporaryX11ErrorHandler);
        }
    }

    ~ScopedX11ErrorTrap()
    {
        if (m_display) {
            XSync(m_display, False);
            XSetErrorHandler(m_previousHandler);
        }
    }

    bool hasError() const
    {
        return g_x11LastErrorCode != 0;
    }

    int errorCode() const
    {
        return g_x11LastErrorCode;
    }

    int requestCode() const
    {
        return g_x11LastRequestCode;
    }

    int minorCode() const
    {
        return g_x11LastMinorCode;
    }

private:
    Display* m_display = nullptr;
    int (*m_previousHandler)(Display*, XErrorEvent*) = nullptr;
};

static int trailingZeroBits(unsigned long mask)
{
    if (mask == 0) {
        return 0;
    }

    int shift = 0;
    while (((mask >> shift) & 1UL) == 0UL) {
        ++shift;
    }
    return shift;
}

static int countMaskBits(unsigned long mask)
{
    int count = 0;
    while (mask != 0) {
        count += static_cast<int>(mask & 1UL);
        mask >>= 1;
    }
    return count;
}

static int normalizeChannel(unsigned long value, int bits)
{
    if (bits <= 0) {
        return 0;
    }

    const unsigned long maxValue = (1UL << bits) - 1UL;
    if (maxValue == 0) {
        return 0;
    }

    return static_cast<int>((value * 255UL) / maxValue);
}

static QImage xImageToQImage(XImage* ximage)
{
    if (!ximage || !ximage->data || ximage->width <= 0 || ximage->height <= 0) {
        return {};
    }

    QImage out(ximage->width, ximage->height, QImage::Format_ARGB32);
    if (out.isNull()) {
        return {};
    }

    const int rShift = trailingZeroBits(ximage->red_mask);
    const int gShift = trailingZeroBits(ximage->green_mask);
    const int bShift = trailingZeroBits(ximage->blue_mask);

    const int rBits = countMaskBits(ximage->red_mask);
    const int gBits = countMaskBits(ximage->green_mask);
    const int bBits = countMaskBits(ximage->blue_mask);

    for (int y = 0; y < ximage->height; ++y) {
        QRgb* dst = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < ximage->width; ++x) {
            const unsigned long pixel = XGetPixel(ximage, x, y);

            const unsigned long r = (pixel & ximage->red_mask) >> rShift;
            const unsigned long g = (pixel & ximage->green_mask) >> gShift;
            const unsigned long b = (pixel & ximage->blue_mask) >> bShift;

            dst[x] = qRgba(
              normalizeChannel(r, rBits),
              normalizeChannel(g, gBits),
              normalizeChannel(b, bBits),
              255);
        }
    }

    return out;
}

} // namespace
#endif

bool ScreenCaptureBackend::isWaylandSession()
{
    const QByteArray waylandDisplay = qgetenv("WAYLAND_DISPLAY");
    const QByteArray sessionType = qgetenv("XDG_SESSION_TYPE").toLower();

    return !waylandDisplay.isEmpty() || sessionType == "wayland";
}

bool ScreenCaptureBackend::isX11Session()
{
    const QByteArray display = qgetenv("DISPLAY");
    const QByteArray sessionType = qgetenv("XDG_SESSION_TYPE").toLower();

    return !display.isEmpty() && sessionType != "wayland";
}

QImage ScreenCaptureBackend::grab(const CaptureRequest& request)
{
    if (!request.rect.isValid() || request.rect.width() <= 0 || request.rect.height() <= 0) {
        qWarning() << "ScreenCaptureBackend::grab rect inválido:" << request.rect;
        return {};
    }

#ifdef Q_OS_LINUX
    return grabWithXGetImage(request);
#else
    Q_UNUSED(request);
    qWarning() << "ScreenCaptureBackend no soportado en esta plataforma.";
    return {};
#endif
}

#ifdef Q_OS_LINUX
QImage ScreenCaptureBackend::grabWithXGetImage(const CaptureRequest& request)
{
    if (!request.display) {
        qWarning() << "ScreenCaptureBackend: Display* es null.";
        return {};
    }

    if (request.target == 0) {
        qWarning() << "ScreenCaptureBackend: target Window es 0.";
        return {};
    }

    const bool wayland = isWaylandSession();

    if (wayland && request.targetIsRoot) {
        qWarning() << "Wayland/Xwayland detectado: XGetImage sobre root no está soportado.";
        return {};
    }

    const int x = request.rect.x();
    const int y = request.rect.y();
    const int w = request.rect.width();
    const int h = request.rect.height();

    qDebug() << "XGetImage con:"
             << "x" << x
             << "y" << y
             << "w" << w
             << "h" << h
             << "targetIsRoot" << request.targetIsRoot
             << "wayland" << wayland;

    ScopedX11ErrorTrap trap(request.display);

    XImage* ximage = XGetImage(
      request.display,
      request.target,
      x,
      y,
      static_cast<unsigned int>(w),
      static_cast<unsigned int>(h),
      AllPlanes,
      ZPixmap);

    XSync(request.display, False);

    if (trap.hasError()) {
        qWarning() << "XGetImage falló. errorCode =" << trap.errorCode()
                   << "requestCode =" << trap.requestCode()
                   << "minorCode =" << trap.minorCode();

        if (ximage) {
            XDestroyImage(ximage);
        }
        return {};
    }

    if (!ximage) {
        qWarning() << "XGetImage devolvió null.";
        return {};
    }

    QImage result = xImageToQImage(ximage);
    XDestroyImage(ximage);

    if (result.isNull()) {
        qWarning() << "No se pudo convertir XImage a QImage.";
    }

    return result;
}
#endif
