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

namespace {
#define PADDING_VALUE 2
}

RectangleTool::RectangleTool(QObject *parent) : AbstractTwoPointTool(parent) {

}

QIcon RectangleTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "square.svg");
}
QString RectangleTool::name() const {
    return tr("Rectangle");
}

QString RectangleTool::nameID() {
    return QLatin1String("");
}

QString RectangleTool::description() const {
    return tr("Set the Rectangle as the paint tool");
}

CaptureTool* RectangleTool::copy(QObject *parent) {
    return new RectangleTool(parent);
}

void RectangleTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.setBrush(QBrush(m_color));
    painter.drawRect(QRect(m_points.first, m_points.second));
}

void RectangleTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void RectangleTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void RectangleTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
