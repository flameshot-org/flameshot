// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QPixmap>
#include <QRect>

/**
 * Captures the X11 root window directly with XGetImage.
 *
 * This is a fallback for servers where XRandR exposes no monitors (virtual or
 * remote displays), so Qt reports a 0x0 screen and QScreen::grabWindow()
 * returns a null pixmap. The implementation lives in a separate translation
 * unit so the X11 macros (Success, KeyPress, ...) do not leak into Qt code.
 */
QPixmap x11RootScreenshot();

/**
 * Returns the geometry of the X11 root window, or an empty rectangle when it
 * cannot be determined. Used to place recording controls when Qt sees no
 * screens.
 */
QRect x11RootGeometry();
