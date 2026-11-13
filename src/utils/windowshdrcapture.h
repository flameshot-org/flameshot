// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QPixmap>

class QScreen;

/**
 * @brief Screen capture for monitors running in Windows HDR mode.
 *
 * When HDR ("Advanced Color") is enabled, Windows composes the desktop in
 * FP16 scRGB, where SDR content sits at the user's SDR white level and video
 * or HDR content can be brighter. QScreen::grabWindow() reads the screen
 * through GDI, which collapses the HDR desktop into an 8-bit image before
 * Flameshot receives it, so Flameshot never sees the scRGB values and has
 * no control over how they are reduced to 8 bits.
 *
 * On those monitors the desktop is instead captured as FP16 scRGB through
 * DXGI Desktop Duplication (IDXGIOutput5::DuplicateOutput1 in
 * R16G16B16A16_FLOAT), and the conversion to SDR is done here, deliberately:
 * values are normalized to the monitor's SDR white level, so SDR white
 * becomes sRGB white and SDR content looks as it does on screen, then
 * clamped to the SDR range (values brighter than SDR white are clipped) and
 * encoded as regular 8-bit sRGB.
 *
 * Monitors that are not in HDR mode never use this path, so SDR users keep
 * the existing GDI capture unchanged.
 */
namespace WindowsHdrCapture {

/**
 * @brief Captures the whole monitor of @p screen when it is in HDR mode.
 *
 * Returns an SDR QImage::Format_RGB32 pixmap with the monitor's native pixel
 * size and the screen's device pixel ratio, like QScreen::grabWindow() does
 * for a full screen grab. Returns a null pixmap when the monitor is not in
 * HDR mode or when anything fails, so the caller can fall back to
 * QScreen::grabWindow().
 */
QPixmap grabScreen(QScreen* screen);

} // namespace
