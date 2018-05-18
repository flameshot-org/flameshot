// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include "abstracttwopointtool.h"

AbstractTwoPointTool::AbstractTwoPointTool(QObject *parent) :
    CaptureTool(parent), m_thickness(0), m_padding(0)
{

}

bool AbstractTwoPointTool::isValid() const {
    return (m_points.first != m_points.second);
}

bool AbstractTwoPointTool::closeOnButtonPressed() const {
    return false;
}

bool AbstractTwoPointTool::isSelectable() const {
    return true;
}

bool AbstractTwoPointTool::showMousePreview() const {
    return true;
}

void AbstractTwoPointTool::undo(QPixmap &pixmap) {
    QPainter p(&pixmap);
    p.drawPixmap(backupRect(pixmap.rect()).topLeft(), m_pixmapBackup);
}

void AbstractTwoPointTool::drawEnd(const QPoint &p) {
    Q_UNUSED(p);
}

void AbstractTwoPointTool::drawMove(const QPoint &p) {
    m_points.second = p;
}

void AbstractTwoPointTool::colorChanged(const QColor &c) {
    m_color = c;
}

void AbstractTwoPointTool::thicknessChanged(const int th) {
    m_thickness = th;
}

void AbstractTwoPointTool::updateBackup(const QPixmap &pixmap) {
    m_pixmapBackup = pixmap.copy(backupRect(pixmap.rect()));
}

QRect AbstractTwoPointTool::backupRect(const QRect &limits) const {
    QRect r = QRect(m_points.first, m_points.second).normalized();
    const int val = m_thickness + m_padding;
    r += QMargins(val, val, val, val);
    return r.intersected(limits);
}
