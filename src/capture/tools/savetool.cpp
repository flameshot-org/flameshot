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

#include "savetool.h"
#include <QPainter>

SaveTool::SaveTool(QObject *parent) : CaptureTool(parent) {

}

int SaveTool::id() const {
    return 0;
}

bool SaveTool::isSelectable() const {
    return false;
}

QString SaveTool::iconName() const {
    return "content-save.png";
}

QString SaveTool::name() const {
    return tr("Save");
}

QString SaveTool::description() const {
    return tr("Save the capture");
}

CaptureTool::ToolWorkType SaveTool::toolType() const {
    return TYPE_WORKER;
}

void SaveTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color,
        const int thickness)
{
    Q_UNUSED(painter);
    Q_UNUSED(points);
    Q_UNUSED(color);
    Q_UNUSED(thickness);
}

void SaveTool::onPressed() {
    Q_EMIT requestAction(REQ_SAVE_SCREENSHOT);
}
