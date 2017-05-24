// Copyright 2017 Alejandro Sirgo Rica
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

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

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType,
                                          void *message, long *result)
{
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
