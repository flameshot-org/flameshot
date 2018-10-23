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

#include "arrowtool.h"

namespace {

#define PADDING_VALUE 2

const int ArrowWidth = 10;
const int ArrowHeight = 18;

QPainterPath getArrowHead(QPoint p1, QPoint p2, const int thickness) {
    QLineF base(p1, p2);
    // Create the vector for the position of the base  of the arrowhead
    QLineF temp(QPoint(0,0), p2-p1);
    int val = ArrowHeight + thickness*4;
    if (base.length() < val) {
        val = (base.length() + thickness*2);
    }
    temp.setLength(base.length() + thickness*2 - val);
    // Move across the line up to the head
    QPointF bottonTranslation(temp.p2());

    // Rotate base of the arrowhead
    base.setLength(ArrowWidth + thickness*2);
    base.setAngle(base.angle() + 90);
    // Move to the correct point
    QPointF temp2 = p1 - base.p2();
    // Center it
    QPointF centerTranslation((temp2.x()/2), (temp2.y()/2));

    base.translate(bottonTranslation);
    base.translate(centerTranslation);

    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(base.p1());
    path.lineTo(base.p2());
    path.lineTo(p2);
    return path;
}

// gets a shorter line to prevent overlap in the point of the arrow
QLine getShorterLine(QPoint p1, QPoint p2, const int thickness) {
    QLineF l(p1, p2);
    int val = ArrowHeight + thickness*4;
    if (l.length() < val) {
        val = (l.length() + thickness*2);
    }
    l.setLength(l.length() + thickness*2 - val);
    return l.toLine();
}

} // unnamed namespace

ArrowTool::ArrowTool(QObject *parent) : AbstractTwoPointTool(parent) {
    m_padding = ArrowWidth / 2;
}

QIcon ArrowTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "arrow-bottom-left.svg");
}
QString ArrowTool::name() const {
    return tr("Arrow");
}

QString ArrowTool::nameID() {
    return QLatin1String("");
}

QString ArrowTool::description() const {
    return tr("Set the Arrow as the paint tool");
}

CaptureTool* ArrowTool::copy(QObject *parent) {
    return new ArrowTool(parent);
}

void ArrowTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawLine(getShorterLine(m_points.first, m_points.second, m_thickness));
    painter.fillPath(getArrowHead(m_points.first, m_points.second, m_thickness), QBrush(m_color));
}

void ArrowTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void ArrowTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void ArrowTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
