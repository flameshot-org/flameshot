// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "circlecounttool.h"
#include <QPainter>
namespace {
#define PADDING_VALUE 2
}

CircleCountTool::CircleCountTool(QObject *parent) : AbstractTwoPointTool(parent) {
    m_count = 0;
}

QIcon CircleCountTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "circlecount-outline.svg");
}
QString CircleCountTool::name() const {
    return tr("Circle Counter");
}

QString CircleCountTool::nameID() {
    return QLatin1String("");
}

QString CircleCountTool::description() const {
    return tr("Add an autoincrementing counter bubble");
}

CaptureTool* CircleCountTool::copy(QObject *parent) {
    return new CircleCountTool(parent);
}

void CircleCountTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setBrush(m_color);

    int bubble_size=16;
    painter.drawEllipse(m_points.first,bubble_size,bubble_size);
    painter.drawText(QRectF(m_points.first.x()-bubble_size/2, m_points.first.y()-bubble_size/2, bubble_size, bubble_size), Qt::AlignCenter, QString::number(m_count));
}

void CircleCountTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void CircleCountTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_count = context.circleCount;
    emit requestAction(REQ_INCREMENT_CIRCLE_COUNT);

}

void CircleCountTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
