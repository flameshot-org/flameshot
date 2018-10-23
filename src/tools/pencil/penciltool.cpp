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

PencilTool::PencilTool(QObject *parent) : AbstractPathTool(parent) {

}

QIcon PencilTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "pencil.svg");
}
QString PencilTool::name() const {
    return tr("Pencil");
}

QString PencilTool::nameID() {
    return QLatin1String("");
}

QString PencilTool::description() const {
    return tr("Set the Pencil as the paint tool");
}

CaptureTool* PencilTool::copy(QObject *parent) {
    return new PencilTool(parent);
}

void PencilTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawPolyline(m_points.data(), m_points.size());
}

void PencilTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    painter.setPen(QPen(context.color, context.thickness + 2));
    painter.drawLine(context.mousePos, context.mousePos);
}

void PencilTool::drawStart(const CaptureContext &context) {
    m_color = context.color;
    m_thickness = context.thickness + 2;
    m_points.append(context.mousePos);
    m_backupArea.setTopLeft(context.mousePos);
    m_backupArea.setBottomRight(context.mousePos);
}

void PencilTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
