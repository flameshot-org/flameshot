// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QPoint>
#include <QRect>

namespace WindowPicker {

/**
 * @brief Rectangle of the top-most window under a global point.
 *
 * The rectangle is the visible outer geometry of the window that contains the
 * point, in the logical coordinates of the screen under it:
 * - X11: the top-level window (direct child of the root window).
 * - Windows: the top-level window, using the DWM extended frame bounds when
 *   available so the invisible resize border and drop shadow are excluded.
 *
 * Windows owned by this process (the capture overlay), input-only windows on
 * X11 and the desktop window itself are ignored.
 *
 * @param globalPos Point in global (screen) coordinates.
 * @return The window rectangle, or a null QRect when no window is there or the
 * platform offers no lookup (macOS, Wayland).
 */
QRect windowRectAt(const QPoint& globalPos);

} // namespace WindowPicker
