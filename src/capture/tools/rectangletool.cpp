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

#include "rectangletool.h"
#include <QPainter>

RectangleTool::RectangleTool(QObject *parent) : CaptureTool(parent) {

}

bool RectangleTool::isSelectable() {
    return true;
}

QString RectangleTool::getIconName() {
    return "square.png";
}

QString RectangleTool::getName() {
    return tr("Rectangle");
}

QString RectangleTool::getDescription() {
    return tr("Sets the Rectangle as the paint tool");
}

CaptureTool::ToolWorkType RectangleTool::getToolType() {
    return TYPE_LINE_DRAWER;
}

void RectangleTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    painter.setPen(QPen(color, 2));
    painter.setBrush(QBrush(color));
    painter.drawRect(QRect(points[0], points[1]));
    painter.setBrush(QBrush());
}

void RectangleTool::onPressed() {
}
