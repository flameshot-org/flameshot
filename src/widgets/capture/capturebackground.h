// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QPainter>
#include <QPixmap>
#include <QRegion>

// The screenshot is immutable while selecting an empty capture. Cache its
// dimmed version and blit only the regions requested by QWidget's paint event.
class CaptureBackground
{
public:
    void paint(QPainter& painter,
               const QPixmap& screenshot,
               const QRegion& brightRegion,
               int opacity)
    {
        if (m_sourceKey != screenshot.cacheKey() || m_opacity != opacity) {
            m_dimmed = screenshot.copy();
            QPainter dimmer(&m_dimmed);
            dimmer.fillRect(
              QRectF(QPointF(), screenshot.deviceIndependentSize()),
              QColor(0, 0, 0, opacity));
            m_sourceKey = screenshot.cacheKey();
            m_opacity = opacity;
        }
        const QRect bounds(QPoint(),
                           screenshot.deviceIndependentSize().toSize());
        painter.save();
        painter.setClipRegion(QRegion(bounds).subtracted(brightRegion),
                              Qt::IntersectClip);
        painter.drawPixmap(0, 0, m_dimmed);
        painter.restore();
        painter.save();
        painter.setClipRegion(brightRegion, Qt::IntersectClip);
        painter.drawPixmap(0, 0, screenshot);
        painter.restore();
    }

private:
    QPixmap m_dimmed;
    qint64 m_sourceKey = 0;
    int m_opacity = -1;
};
