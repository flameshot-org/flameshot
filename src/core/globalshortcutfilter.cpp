// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "globalshortcutfilter.h"
#include "src/core/controller.h"
#include <qt_windows.h>

GlobalShortcutFilter::GlobalShortcutFilter(QObject* parent)
  : QObject(parent)
{
    // Forced Print Screen
    if (RegisterHotKey(NULL, 1, 0, VK_SNAPSHOT)) {
        // ok - capture screen
    }

    if (RegisterHotKey(NULL, 2, MOD_SHIFT, VK_SNAPSHOT)) {
        // ok - show screenshots history
    }
}

bool GlobalShortcutFilter::nativeEventFilter(const QByteArray& eventType,
                                             void* message,
                                             long* result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) {
        // TODO: this is just a temporal workwrround, proper global
        // support would need custom shortcuts defined by the user.
        const quint32 keycode = HIWORD(msg->lParam);
        const quint32 modifiers = LOWORD(msg->lParam);

        // Show screenshots history
        if (VK_SNAPSHOT == keycode && MOD_SHIFT == modifiers) {
            Controller::getInstance()->showRecentScreenshots();
        }

        // Capture screen
        if (VK_SNAPSHOT == keycode && 0 == modifiers) {
            Controller::getInstance()->requestCapture(
              CaptureRequest(CaptureRequest::GRAPHICAL_MODE));
        }

        return true;
    }
    return false;
}
