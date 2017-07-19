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

#include "arrowtool.h"
#include <QPainter>

namespace {
    const int ArrowWidth = 10;
    const int ArrowHeight = 18;

    QPainterPath getArrowHead(QPoint p1, QPoint p2) {
        QLineF body(p1, p2);
        int originalLength = body.length();
        body.setLength(ArrowWidth);
        // move across the line up to the head
        //QPointF  =;
        QLineF temp(QPoint(0,0), p2-p1);
        temp.setLength(originalLength-ArrowHeight);
        QPointF bottonTranslation(temp.p2());

        // generates the transformation to center of the arrowhead
        body.setAngle(body.angle()+90);
        QPointF temp2 = p1-body.p2();
        QPointF centerTranslation((temp2.x()/2), (temp2.y()/2));

        body.translate(bottonTranslation);
        body.translate(centerTranslation);

        QPainterPath path;
        path.moveTo(p2);
        path.lineTo(body.p1());
        path.lineTo(body.p2());
        path.lineTo(p2);
        return path;
    }

    // gets a shorter line to prevent overlap in the point of the arrow
    QLine getShorterLine(QPoint p1, QPoint p2) {
        QLineF l(p1, p2);
        l.setLength(l.length()-ArrowHeight);
        return l.toLine();
    }

}

ArrowTool::ArrowTool(QObject *parent) : CaptureTool(parent) {

}

bool ArrowTool::isSelectable() {
    return true;
}

QString ArrowTool::getIconName() {
    return "arrow-bottom-left.png";
}

QString ArrowTool::getName() {
    return tr("Arrow");
}

QString ArrowTool::getDescription() {
    return tr("Sets the Arrow as the paint tool");
}

CaptureTool::ToolWorkType ArrowTool::getToolType() {
    return TYPE_LINE_DRAWER;
}

void ArrowTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    painter.setPen(QPen(color, 2));
    painter.drawLine(getShorterLine(points[0], points[1]));
    painter.fillPath(getArrowHead(points[0], points[1]), QBrush(color));
}

void ArrowTool::onPressed() {
}
