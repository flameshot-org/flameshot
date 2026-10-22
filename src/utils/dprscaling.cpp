// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprscaling.h"

namespace DprScaling {

Fit fitCrop(const QSize& cropped, const QSize& logical, qreal reportedDpr)
{
    if (logical.width() <= 0 || logical.height() <= 0) {
        return { cropped, reportedDpr };
    }

    // Qt rounds a compositor's scale up, never down, so this over-estimates the
    // monitor's real pixels rather than under-estimating them. Clamping to it
    // therefore cannot throw away pixels the panel could have shown.
    const int budgetWidth = qRound(logical.width() * reportedDpr);
    const int budgetHeight = qRound(logical.height() * reportedDpr);

    QSize size = cropped;
    if (cropped.width() > budgetWidth || cropped.height() > budgetHeight) {
        size = QSize(budgetWidth, budgetHeight);
    }

    return { size, qreal(size.width()) / logical.width() };
}

QRect toDevicePixels(const QRect& logical, qreal dpr)
{
    // width() is the distance to the exclusive far edge, so scaling that edge
    // and subtracting gives an extent that always reaches it.
    const int left = qRound(logical.left() * dpr);
    const int top = qRound(logical.top() * dpr);
    const int right = qRound((logical.left() + logical.width()) * dpr);
    const int bottom = qRound((logical.top() + logical.height()) * dpr);

    return { left, top, right - left, bottom - top };
}

int toDevicePixels(qreal logical, qreal dpr)
{
    return qRound(logical * dpr);
}

QPoint toPixmapPoint(const QPoint& global, const QPoint& origin, qreal dpr)
{
    return { qRound((global.x() - origin.x()) * dpr),
             qRound((global.y() - origin.y()) * dpr) };
}

} // namespace DprScaling
