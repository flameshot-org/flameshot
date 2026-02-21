// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "screengrabber_geometry.h"
#include <QtMath>
#include <algorithm>

namespace ScreenGeometry {

// Calculate total logical dimensions and minimum coordinates
static void logicalBounds(const QList<ScreenMetadata>& screens,
                          int& minX,
                          int& minY,
                          int& maxX,
                          int& maxY)
{
    minX = 0;
    minY = 0;
    maxX = 0;
    maxY = 0;
    for (const auto& s : screens) {
        minX = qMin(minX, s.geometry.x());
        minY = qMin(minY, s.geometry.y());
        maxX = qMax(maxX, s.geometry.x() + s.geometry.width());
        maxY = qMax(maxY, s.geometry.y() + s.geometry.height());
    }
}

// Linux (both X11 and Wayland via freedesktop portal):
// Use logical coordinate-based cropping since portal returns full desktop
QRect calculateCropRectLinux(const QList<ScreenMetadata>& screens,
                             int monitorIndex,
                             int screenshotWidth,
                             int screenshotHeight)
{
    if (monitorIndex < 0 || monitorIndex >= screens.size()) {
        return QRect();
    }

    const ScreenMetadata& target = screens[monitorIndex];

    int minX, minY, maxX, maxY;
    logicalBounds(screens, minX, minY, maxX, maxY);

    int totalLogicalWidth = maxX - minX;
    int totalLogicalHeight = maxY - minY;

    if (totalLogicalWidth <= 0 || totalLogicalHeight <= 0) {
        return QRect();
    }

    // Screenshot scale factors
    qreal scaleX = static_cast<qreal>(screenshotWidth) / totalLogicalWidth;
    qreal scaleY = static_cast<qreal>(screenshotHeight) / totalLogicalHeight;

    int cropX = qRound((target.geometry.x() - minX) * scaleX);
    int cropY = qRound((target.geometry.y() - minY) * scaleY);
    int cropW = qRound(target.geometry.width() * scaleX);
    int cropH = qRound(target.geometry.height() * scaleY);

    return QRect(cropX, cropY, cropW, cropH);
}

// Windows: Calculate physical pixel positions for mixed DPI
QRect calculateCropRectWindows(const QList<ScreenMetadata>& screens,
                               int monitorIndex)
{
    if (monitorIndex < 0 || monitorIndex >= screens.size()) {
        return QRect();
    }

    const ScreenMetadata& target = screens[monitorIndex];

    int cropX = 0;
    int cropY = 0;

    for (const auto& s : screens) {
        // Sum physical widths of screens completely to the left
        if (s.geometry.x() + s.geometry.width() <= target.geometry.x()) {
            cropX += qRound(s.geometry.width() * s.dpr);
        }
        // Sum physical heights of screens completely above
        if (s.geometry.y() + s.geometry.height() <= target.geometry.y()) {
            cropY += qRound(s.geometry.height() * s.dpr);
        }
    }

    int cropW = qRound(target.geometry.width() * target.dpr);
    int cropH = qRound(target.geometry.height() * target.dpr);

    return QRect(cropX, cropY, cropW, cropH);
}

QRect calculateDesktopGeometry(const QList<ScreenMetadata>& screens,
                               bool isWindows)
{
    QRect geometry;
    for (const auto& s : screens) {
        QRect scrRect = s.geometry;
        if (!isWindows) {
            // https://doc.qt.io/qt-6/highdpi.html#device-independent-screen-geometry
            scrRect.moveTo(
              QPointF(scrRect.x() / s.dpr, scrRect.y() / s.dpr).toPoint());
        }

        geometry = geometry.united(scrRect);
    }

    return geometry;
}

QList<int> sortScreenIndicesByPosition(const QList<ScreenMetadata>& screens)
{
    QList<int> indices;
    indices.reserve(screens.size());
    for (int i = 0; i < screens.size(); ++i) {
        indices.append(i);
    }

    std::sort(indices.begin(), indices.end(), [&screens](int a, int b) {
        return screens[a].geometry.x() < screens[b].geometry.x();
    });

    return indices;
}

// Linux: May need rescaling if scale factors don't match
bool linuxCropNeedsRescaling(const QList<ScreenMetadata>& screens,
                             int monitorIndex,
                             int screenshotWidth)
{
    if (monitorIndex < 0 || monitorIndex >= screens.size()) {
        return false;
    }

    int minX, minY, maxX, maxY;
    logicalBounds(screens, minX, minY, maxX, maxY);

    int totalLogicalWidth = maxX - minX;
    if (totalLogicalWidth <= 0) {
        return false;
    }

    qreal scaleX = static_cast<qreal>(screenshotWidth) / totalLogicalWidth;
    return qAbs(scaleX - screens[monitorIndex].dpr) > 0.01;
}
}
