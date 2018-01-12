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

#include "penciltool.h"
#include <QPainter>

PencilTool::PencilTool(QObject *parent) : CaptureTool(parent) {

}

int PencilTool::id() const {
    return 0;
}

bool PencilTool::isSelectable() const {
    return true;
}

QString PencilTool::iconName() const {
    return "pencil.png";
}

QString PencilTool::name() const {
    return tr("Pencil");
}

QString PencilTool::description() const {
    return tr("Sets the Pencil as the paint tool");
}

CaptureTool::ToolWorkType PencilTool::toolType() const {
    return TYPE_PATH_DRAWER;
}

void PencilTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color,
        const int thickness)
{
    painter.setPen(QPen(color, 2 + thickness));
    if (points.length() == 2) {
        painter.drawLine(points[0], points[1]);
    } else {
        painter.drawPolyline(points.data(), points.size());
    }
}

void PencilTool::onPressed() {
}
