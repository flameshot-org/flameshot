// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "windowsforeground.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace {
HWND handle(WId window)
{
    return reinterpret_cast<HWND>(window);
}
}

bool WindowsForeground::isForeground(WId window)
{
    return GetForegroundWindow() == handle(window);
}

bool WindowsForeground::take(WId window)
{
    const HWND target = handle(window);
    if (GetForegroundWindow() == target) {
        return true;
    }
    // Allowed while the global shortcut that started the capture still
    // grants the right to activate.
    SetForegroundWindow(target);
    if (GetForegroundWindow() == target) {
        return true;
    }

    // Otherwise share the foreground thread's input state for a moment, as
    // Qt does when it is asked to always activate a window.
    const HWND foreground = GetForegroundWindow();
    const DWORD self = GetCurrentThreadId();
    const DWORD other =
      foreground ? GetWindowThreadProcessId(foreground, nullptr) : 0;
    DWORD_PTR unused = 0;
    if (other && other != self &&
        SendMessageTimeoutW(foreground,
                            WM_NULL,
                            0,
                            0,
                            SMTO_ABORTIFHUNG | SMTO_BLOCK,
                            200,
                            &unused) &&
        AttachThreadInput(other, self, TRUE)) {
        BringWindowToTop(target);
        SetForegroundWindow(target);
        SetFocus(target);
        AttachThreadInput(other, self, FALSE);
    }
    return GetForegroundWindow() == target;
}

QString WindowsForeground::describe()
{
    const HWND foreground = GetForegroundWindow();
    wchar_t name[128] = {};
    GetClassNameW(foreground, name, 128);
    DWORD pid = 0;
    GetWindowThreadProcessId(foreground, &pid);
    return QStringLiteral("hwnd=0x%1 class=%2 pid=%3")
      .arg(reinterpret_cast<quintptr>(foreground), 0, 16)
      .arg(QString::fromWCharArray(name))
      .arg(pid);
}
