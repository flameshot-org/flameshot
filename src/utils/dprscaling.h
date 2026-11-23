// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPoint>
#include <QRect>
#include <QSize>

/// Arithmetic reconciling the scale factors a capture passes through: the
/// compositor's, the one Qt reports for a screen, and the one the screenshot
/// actually arrives at. QtCore only, so it is unit-testable headless.
namespace DprScaling {

struct Fit
{
    QSize size;
    qreal dpr;
};

/// Pixel size and device pixel ratio for one monitor's crop taken out of a
/// composited desktop grab.
///
/// @param cropped     what the crop actually holds, in pixels
/// @param logical     the monitor's logical geometry
/// @param reportedDpr Qt's devicePixelRatio for that monitor
///
/// Never enlarges: a grab cannot gain detail it did not capture, and scaling up
/// to match a reported ratio only invents pixels. Shrinks only when the crop
/// holds more than the monitor can, which is safe because Qt rounds a
/// compositor's scale up, making `logical * reportedDpr` an upper bound on the
/// monitor's real pixels. The returned ratio describes the pixels that remain,
/// so the crop keeps its logical size either way.
Fit fitCrop(const QSize& cropped, const QSize& logical, qreal reportedDpr);

/// A logical rectangle in device pixels.
///
/// Scales the edges and derives the extent from them, rather than scaling
/// origin and extent apart: done separately the two round independently and the
/// far edge drifts by a pixel, which at fractional ratios leaves gaps between
/// rectangles that were adjacent. At whole-number ratios this is exactly
/// origin and extent scaled directly.
QRect toDevicePixels(const QRect& logical, qreal dpr);

/// A logical length in device pixels. Takes a real length because callers
/// divide by a display's scale first, which rarely lands on a whole number.
int toDevicePixels(qreal logical, qreal dpr);

/// Where a point on the desktop falls inside one monitor's capture.
///
/// @param global logical position in desktop coordinates
/// @param origin logical top-left of the monitor the capture came from
/// @param dpr    ratio of the capture, not of the screen: the two disagree
///               wherever a compositor scales fractionally
QPoint toPixmapPoint(const QPoint& global, const QPoint& origin, qreal dpr);

} // namespace DprScaling
