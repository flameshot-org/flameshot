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

#include "penciltool.h"
#include <QPainter>

PencilTool::PencilTool(QObject *parent) : CaptureTool(parent) {

}

bool PencilTool::isSelectable() {
    return true;
}

QString PencilTool::getIconName() {
    return "pencil.png";
}

QString PencilTool::getName() {
    return tr("Pencil");
}

QString PencilTool::getDescription() {
    return tr("Sets the Pencil as the paint tool");
}

CaptureTool::ToolWorkType PencilTool::getToolType() {
    return TYPE_PATH_DRAWER;
}

void PencilTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    painter.setPen(QPen(color, 2));
    painter.drawPolyline(points.data(), points.size());
}

void PencilTool::onPressed() {
}
