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

#include "redotool.h"
#include <QPainter>

RedoTool::RedoTool(QObject *parent) : AbstractActionTool(parent) {

}

bool RedoTool::closeOnButtonPressed() const {
    return false;
}

QIcon RedoTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "redo-variant.svg");
}
QString RedoTool::name() const {
    return tr("Redo");
}

QString RedoTool::nameID() {
    return QLatin1String("");
}

QString RedoTool::description() const {
    return tr("Redo the next modification");
}

CaptureTool* RedoTool::copy(QObject *parent) {
    return new RedoTool(parent);
}

void RedoTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
    emit requestAction(REQ_REDO_MODIFICATION);
}
