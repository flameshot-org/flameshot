#include "nativeeventfilter.h"
#include <QVector>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>

namespace {
    Display * display;        // Connection to X11
    Window win;               // Grab window
    int keycode;
    QVector<quint32> maskModifiers;
}

NativeEventFilter::NativeEventFilter(QObject *parent) : QObject(parent) {
    display = QX11Info::display();
    win = DefaultRootWindow(display);
    maskModifiers << 0 << Mod2Mask << LockMask << (Mod2Mask | LockMask);
    setShortcut();
}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    xcb_key_press_event_t *keyEvent;

    // Check xcb event
    if (eventType == "xcb_generic_event_t") {
        // cast message xcb event
        xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);

        // check key press
        if ((event->response_type & 127) == XCB_KEY_PRESS){

            keyEvent = static_cast<xcb_key_press_event_t *>(message);

            foreach (quint32 maskMods, maskModifiers) {
                if((keyEvent->state == maskMods)
                        &&  keyEvent->detail == keycode){
                    emit activated();
                    return true;
                }
            }
        }
    }
    return false;
}

void NativeEventFilter::setShortcut() {
    keycode = XKeysymToKeycode(display, XK_Print);
    foreach (quint32 maskMods, maskModifiers) {
        XGrabKey(display,
                 keycode ,
                 maskMods,
                 win,
                 True,
                 GrabModeAsync,
                 GrabModeAsync);
    }
}
