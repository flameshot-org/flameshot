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

AbstractPathTool::AbstractPathTool(QObject *parent)  :
    CaptureTool(parent), m_thickness(0), m_padding(0)
{

}

bool AbstractPathTool::isValid() const {
    return m_points.length() > 1;
}

bool AbstractPathTool::closeOnButtonPressed() const {
    return false;
}

bool AbstractPathTool::isSelectable() const {
    return true;
}

bool AbstractPathTool::showMousePreview() const {
    return true;
}

void AbstractPathTool::undo(QPixmap &pixmap) {
    QPainter p(&pixmap);
    const int val = m_thickness + m_padding;
    QRect area = m_backupArea + QMargins(val, val, val, val);
    p.drawPixmap(area.intersected(pixmap.rect())
                 .topLeft(), m_pixmapBackup);
}

void AbstractPathTool::drawEnd(const QPoint &p) {
    Q_UNUSED(p);
}

void AbstractPathTool::drawMove(const QPoint &p) {
    addPoint(p);
}

void AbstractPathTool::colorChanged(const QColor &c) {
    m_color = c;
}

void AbstractPathTool::thicknessChanged(const int th) {
    m_thickness = th;
}

void AbstractPathTool::updateBackup(const QPixmap &pixmap) {
    const int val = m_thickness + m_padding;
    QRect area = m_backupArea.normalized() + QMargins(val, val, val, val);
    m_pixmapBackup = pixmap.copy(area);
}

void AbstractPathTool::addPoint(const QPoint &point) {
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
