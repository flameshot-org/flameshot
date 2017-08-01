// Copyright 2017 Alejandro Sirgo Rica
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

#include "markertool.h"
#include <QPainter>

#define ADJ_VALUE 14

MarkerTool::MarkerTool(QObject *parent) : CaptureTool(parent) {

}

bool MarkerTool::isSelectable() {
    return true;
}

QString MarkerTool::getIconName() {
    return "marker.png";
}

QString MarkerTool::getName() {
    return tr("Marker");
}

QString MarkerTool::getDescription() {
    return tr("Sets the Marker as the paint tool");
}

CaptureTool::ToolWorkType MarkerTool::getToolType() {
    return TYPE_LINE_DRAWER;
}

void MarkerTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    QPoint p0 = points[0];
    QPoint p1 = points[1];
    if (needsAdjustment(p0, p1)) {
        p1.setY(p0.y());
    }
    painter.setOpacity(0.35);
    painter.setPen(QPen(color, 14));
    painter.drawLine(p0, p1);
    painter.setOpacity(1);
}

void MarkerTool::onPressed() {
}

bool MarkerTool::needsAdjustment(const QPoint &p0, const QPoint &p1) const {
    return (p1.y() >= p0.y() - ADJ_VALUE) && (p1.y() <= p0.y() + ADJ_VALUE);
}
