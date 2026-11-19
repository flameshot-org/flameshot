// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QString>
#include <QWindow>

// Windows may refuse to bring a window shown by a background application to
// the front, even right after its global shortcut. Qt then still reports the
// window as active, but keyboard input goes to another application.
namespace WindowsForeground {
// Tries to bring the window to the front. Returns whether it is there.
bool take(WId window);
// Whether the window is the one that receives keyboard input.
bool isForeground(WId window);
// The foreground window's class and process, for diagnostics.
QString describe();
}
