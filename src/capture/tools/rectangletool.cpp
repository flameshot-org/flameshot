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

#include "rectangletool.h"
#include <QPainter>

RectangleTool::RectangleTool(QObject *parent) : CaptureTool(parent) {

}

int RectangleTool::id() const {
    return 0;
}

bool RectangleTool::isSelectable() const {
    return true;
}

QString RectangleTool::iconName() const {
    return "square.png";
}

QString RectangleTool::name() const {
    return tr("Rectangle");
}

QString RectangleTool::description() const {
    return tr("Sets the Rectangle as the paint tool");
}

CaptureTool::ToolWorkType RectangleTool::toolType() const {
    return TYPE_LINE_DRAWER;
}

void RectangleTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color,
        const int thickness)
{
    painter.setPen(QPen(color, 2  + thickness));
    painter.setBrush(QBrush(color));
    painter.drawRect(QRect(points[0], points[1]));
    painter.setBrush(QBrush());
}

void RectangleTool::onPressed() {
}
