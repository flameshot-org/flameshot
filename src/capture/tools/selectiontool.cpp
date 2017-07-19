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

#include "selectiontool.h"
#include <QPainter>

SelectionTool::SelectionTool(QObject *parent) : CaptureTool(parent) {

}

bool SelectionTool::isSelectable() {
    return true;
}

QString SelectionTool::getIconName() {
    return "square-outline.png";
}

QString SelectionTool::getName() {
    return tr("Rectangular Selection");
}

QString SelectionTool::getDescription() {
    return tr("Sets the Selection as the paint tool");
}

CaptureTool::ToolWorkType SelectionTool::getToolType() {
    return TYPE_LINE_DRAWER;
}

void SelectionTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    painter.setPen(QPen(color, 2));
    painter.drawRect(QRect(points[0], points[1]));
}

void SelectionTool::onPressed() {
}
