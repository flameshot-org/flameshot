#include "overlaytool.h"
#include <QPainter>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QKeyEvent>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>


#include <X11/extensions/shape.h>
//#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

#include <QApplication>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <qnativeinterface.h>
#else
#include <QX11Info>
#endif



Overlay::Overlay(QWidget* parent)
  : QWidget(parent), lastRect(QRect()) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
    setFixedSize(QApplication::primaryScreen()->size());

    qDebug() << "Overlay comienza0000000000000...";
}

void Overlay::updateOverlay(const QRect& rect) {
    lastRect = rect;
    update();
}

void Overlay::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 3));
    painter.drawRect(lastRect);
    qDebug() << "Overlay comienza111111111...";
}



WId Overlay::getRealWindowUnderCursor(WId overlayWinId)
{
    Display* display = XOpenDisplay(nullptr);

    Window root = DefaultRootWindow(display);
    Window returnedRoot, returnedParent;
    Window *children = nullptr;
    unsigned int nchildren = 0;

    int x, y;
    unsigned int mask;
    Window dummy;

    if (!XQueryPointer(display, root, &dummy, &dummy, &x, &y, &x, &y, &mask)) {
        qWarning() << "❌ No se pudo obtener el puntero";
        return 0;
    }

    if (!XQueryTree(display, root, &returnedRoot, &returnedParent, &children, &nchildren)) {
        qWarning() << "❌ No se pudo consultar jerarquía de ventanas";
        return 0;
    }

    WId result = 0;
    for (int i = static_cast<int>(nchildren) - 1; i >= 0; --i) {
        if (children[i] == overlayWinId) continue;

        XWindowAttributes attr;
        if (!XGetWindowAttributes(display, children[i], &attr)) continue;
        if (attr.map_state != IsViewable) continue;

        if (x >= attr.x && x <= (attr.x + attr.width) &&
            y >= attr.y && y <= (attr.y + attr.height)) {
            result = children[i];
            break;
        }
    }

    if (children)
        XFree(children);

    return result;
}

void Overlay::startClickDetection()
{
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        qWarning() << "❌ No se pudo abrir X11 display";
        return;
    }

    Window root = DefaultRootWindow(display);

           // Cambiar cursor opcionalmente
    Cursor cursor = XCreateFontCursor(display, XC_crosshair);
    int grabResult = XGrabPointer(display, root, True,
                                  ButtonPressMask, GrabModeAsync, GrabModeAsync,
                                  None, cursor, CurrentTime);

    if (grabResult != GrabSuccess) {
        qWarning() << "❌ No se pudo capturar el puntero del mouse";
        return;
    }

    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == ButtonPress) {
            // Al hacer clic, obtenemos la ventana válida
            WId overlayId = this->winId();
            WId selectedWindow = getRealWindowUnderCursor(overlayId);

            if (selectedWindow != 0) {
                qDebug() << "✅ Ventana seleccionada:" << selectedWindow;

                emit windowSelected(selectedWindow);  // Tu señal, o úsalo directamente

                break;
            } else {
                qWarning() << "❌ No se encontró ventana válida";
            }
        }
    }

    XUngrabPointer(display, CurrentTime);
    XFreeCursor(display, cursor);
    XCloseDisplay(display);
}

void Overlay::mousePressEvent(QMouseEvent* event) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;

    Window root = DefaultRootWindow(display);
    Window retRoot, retChild;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    XQueryPointer(display, root, &retRoot, &retChild, &rootX, &rootY, &winX, &winY, &mask);
    //XCloseDisplay(display);

    emit windowSelected(static_cast<WId>(retChild));
    close();
}

void Overlay::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        close();
    }
}
