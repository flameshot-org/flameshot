// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "abstractpathtool.h"

AbstractPathTool::AbstractPathTool(QObject* parent)
  : CaptureTool(parent)
  , m_thickness(0)
  , m_padding(0)
{}

bool AbstractPathTool::isValid() const
{
    return m_points.length() > 1;
}

bool AbstractPathTool::closeOnButtonPressed() const
{
    return false;
}

bool AbstractPathTool::isSelectable() const
{
    return true;
}

bool AbstractPathTool::showMousePreview() const
{
    return true;
}

void AbstractPathTool::undo(QPixmap& pixmap)
{
    QPainter p(&pixmap);
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    // Not sure how will it work on 4k and fullHd on Linux or Windows with a
    // capture of different displays with different DPI, so let it be MacOS
    // specific only.
    const qreal pixelRatio = pixmap.devicePixelRatio();
    p.drawPixmap(backupRect(pixmap).topLeft() / pixelRatio, m_pixmapBackup);
#else
    p.drawPixmap(backupRect(pixmap).topLeft(), m_pixmapBackup);
#endif
}

void AbstractPathTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p);
}

void AbstractPathTool::drawMove(const QPoint& p)
{
    addPoint(p);
}

void AbstractPathTool::colorChanged(const QColor& c)
{
    m_color = c;
}

void AbstractPathTool::thicknessChanged(const int th)
{
    m_thickness = th;
}

void AbstractPathTool::updateBackup(const QPixmap& pixmap)
{
    m_pixmapBackup = pixmap.copy(backupRect(pixmap));
}

QRect AbstractPathTool::backupRect(const QPixmap& pixmap) const
{
    const QRect& limits = pixmap.rect();
#if (defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACOS) ||        \
     defined(Q_OS_MACX))
    // Not sure how will it work on 4k and fullHd on Linux or Windows with a
    // capture of different displays with different DPI, so let it be MacOS
    // specific only.
    const qreal pixelRatio = pixmap.devicePixelRatio();
    const int val = (m_thickness + m_padding) * pixelRatio;
    QRect r = m_backupArea.normalized();
    if (1 != pixelRatio) {
        r.moveTo(r.topLeft() * pixelRatio);
        r.setSize(r.size() * pixelRatio);
    }
#else
    const int val = m_thickness + m_padding;
    QRect r = m_backupArea.normalized();
#endif
    r += QMargins(val, val, val, val);
    return r.intersected(limits);
}

void AbstractPathTool::addPoint(const QPoint& point)
{
    if (m_backupArea.left() > point.x()) {
        m_backupArea.setLeft(point.x());
    } else if (m_backupArea.right() < point.x()) {
        m_backupArea.setRight(point.x());
    }
    if (m_backupArea.top() > point.y()) {
        m_backupArea.setTop(point.y());
    } else if (m_backupArea.bottom() < point.y()) {
        m_backupArea.setBottom(point.y());
    }
    m_points.append(point);
}
