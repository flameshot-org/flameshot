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

#include "linetool.h"
#include <QPainter>

namespace {

#define ADJ_VALUE 13
#define PADDING_VALUE 2

// Have to force horizontal position
bool needsAdjustment(const QPoint &p0, const QPoint &p1) {
    return (p1.y() >= p0.y() - ADJ_VALUE) && (p1.y() <= p0.y() + ADJ_VALUE);
}

}

LineTool::LineTool(QObject *parent) : AbstractTwoPointTool(parent) {

}

QIcon LineTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "line.svg");
}
QString LineTool::name() const {
    return tr("Line");
}

QString LineTool::nameID() {
    return QLatin1String("");
}

QString LineTool::description() const {
    return tr("Set the Line as the paint tool");
}

CaptureTool* LineTool::copy(QObject *parent) {
    return new LineTool(parent);
}

void LineTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawLine(m_points.first, m_points.second);
}

void LineTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void LineTool::drawMove(const QPoint &p) {
    m_points.second = p;
    if (needsAdjustment(m_points.first, m_points.second)) {
        m_points.second.setY(m_points.first.y());
    }
}

void LineTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void LineTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
