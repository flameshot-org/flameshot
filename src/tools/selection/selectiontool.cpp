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

#include "selectiontool.h"
#include <QPainter>

namespace {
#define PADDING_VALUE 2
}

SelectionTool::SelectionTool(QObject *parent) : AbstractTwoPointTool(parent) {

}

bool SelectionTool::closeOnButtonPressed() const {
    return false;
}

QIcon SelectionTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "square-outline.svg");
}
QString SelectionTool::name() const {
    return tr("Rectangular Selection");
}

QString SelectionTool::nameID() {
    return QLatin1String("");
}

QString SelectionTool::description() const {
    return tr("Set Selection as the paint tool");
}

CaptureTool* SelectionTool::copy(QObject *parent) {
    return new SelectionTool(parent);
}

void SelectionTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawRect(QRect(m_points.first, m_points.second));
}

void SelectionTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void SelectionTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void SelectionTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
