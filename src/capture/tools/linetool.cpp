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

#include "linetool.h"
#include <QPainter>

LineTool::LineTool(QObject *parent) : CaptureTool(parent) {

}

bool LineTool::isSelectable() {
    return true;
}

QString LineTool::getIconName() {
    return "line.png";
}

QString LineTool::getName() {
    return tr("Line");
}

QString LineTool::getDescription() {
    return tr("Sets the Line as the paint tool");
}

CaptureTool::ToolWorkType LineTool::getToolType() {
    return TYPE_LINE_DRAWER;
}

void LineTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    painter.setPen(QPen(color, 2));
    painter.drawLine(points[0], points[1]);
}

void LineTool::onPressed() {
}
