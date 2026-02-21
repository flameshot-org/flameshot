// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QList>
#include <QRect>
#include <QtGlobal>

struct ScreenMetadata
{
    QRect geometry; // logical geometry from QScreen::geometry()
    qreal dpr;      // device pixel ratio from QScreen::devicePixelRatio()
};

namespace ScreenGeometry {
// Compute crop rectangle using logical coordinate scaling.
// Used on Linux where the freedesktop portal returns a full-desktop image
// and we need to map logical monitor positions to screenshot pixel positions.
QRect calculateCropRectLinux(const QList<ScreenMetadata>& screens,
                             int monitorIndex,
                             int screenshotWidth,
                             int screenshotHeight);

// Compute crop rectangle using physical pixel positions.
// Used on Windows where each screen is captured at its native DPI and
// composited into a canvas whose layout is in physical pixels.
QRect calculateCropRectWindows(const QList<ScreenMetadata>& screens,
                               int monitorIndex);

// Compute the unified bounding rectangle of all screens.
// When isWindows is false, screen positions are divided by DPR to convert
// to device-independent coordinates (matching Qt's high-DPI behavior).
QRect calculateDesktopGeometry(const QList<ScreenMetadata>& screens,
                               bool isWindows);

// Return screen indices sorted by X position (left to right).
QList<int> sortScreenIndicesByPosition(const QList<ScreenMetadata>& screens);

// Whether the Linux crop needs post-crop rescaling to match target DPR.
// Returns true when the screenshot's effective scale differs from the
// target monitor's DPR by more than 0.01.
bool linuxCropNeedsRescaling(const QList<ScreenMetadata>& screens,
                             int monitorIndex,
                             int screenshotWidth);
}
