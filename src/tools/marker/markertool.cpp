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

#include "markertool.h"
#include <QPainter>

namespace {

#define ADJ_VALUE 14
#define PADDING_VALUE 14

// Have to force horizontal position
bool needsHorizontalAdjustment(const QPoint &p0, const QPoint &p1) {
    return (p1.y() >= p0.y() - ADJ_VALUE) && (p1.y() <= p0.y() + ADJ_VALUE);
}

// Have to force vertical position
bool needsVerticalAdjustment(const QPoint &p0, const QPoint &p1) {
    return (p1.x() >= p0.x() - ADJ_VALUE) && (p1.x() <= p0.x() + ADJ_VALUE);
}

// Have to force one of the four possible 45-degree direction positions
bool needsDiagonalAdjustment(const QPoint &p0, const QPoint &p1) {
    return ((p1.x() + p1.y() - p0.x() - p0.y()) *
            (p1.x() + p1.y() - p0.x() - p0.y()) <= 2 * ADJ_VALUE * ADJ_VALUE) ||
            ((p1.x() - p1.y() + p0.y() - p0.x()) *
            (p1.x() - p1.y() + p0.y() - p0.x()) <= 2 * ADJ_VALUE * ADJ_VALUE);
}

}

MarkerTool::MarkerTool(QObject *parent) : AbstractTwoPointTool(parent) {

}

QIcon MarkerTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "marker.svg");
}
QString MarkerTool::name() const {
    return tr("Marker");
}

QString MarkerTool::nameID() {
    return QLatin1String("");
}

QString MarkerTool::description() const {
    return tr("Set the Marker as the paint tool");
}

CaptureTool* MarkerTool::copy(QObject *parent) {
    return new MarkerTool(parent);
}

void MarkerTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setOpacity(0.35);
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawLine(m_points.first, m_points.second);
}

void MarkerTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setOpacity(0.35);
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void MarkerTool::drawMove(const QPoint &p) {
    m_points.second = p;
    if (needsHorizontalAdjustment(m_points.first, m_points.second)) {
        m_points.second.setY(m_points.first.y());
    } else if (needsVerticalAdjustment(m_points.first, m_points.second)) {
        m_points.second.setX(m_points.first.x());
    } else if (needsDiagonalAdjustment(m_points.first, m_points.second)) {
        const QPoint* p0 = &m_points.first;
        QPoint* p1 = &m_points.second;
        if ((p1->x() >= p0->x()) == (p1->y() >= p0->y())) {
            int newY = (p1->x() + p1->y() - p0->x() + p0->y()) / 2;
            int newX = (p1->x() + p1->y() + p0->x() - p0->y()) / 2;
            p1->setX(newX);
            p1->setY(newY);
        } else {
            int newX = (p0->x() + p0->y() + p1->x() - p1->y()) / 2;
            int newY = p0->x() + p0->y() - newX;
            p1->setX(newX);
            p1->setY(newY);
        }
    }
}

void MarkerTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void MarkerTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}

void MarkerTool::thicknessChanged(const int th) {
    m_thickness = th + PADDING_VALUE;
}
