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

#include "exittool.h"
#include <QPainter>

ExitTool::ExitTool(QObject *parent) : AbstractActionTool(parent) {

}

bool ExitTool::closeOnButtonPressed() const {
    return true;
}

QIcon ExitTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "close.svg");
}
QString ExitTool::name() const {
    return tr("Exit");
}

QString ExitTool::nameID() {
    return QLatin1String("");
}

QString ExitTool::description() const {
    return tr("Leave the capture screen");
}

CaptureTool* ExitTool::copy(QObject *parent) {
    return new ExitTool(parent);
}

void ExitTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
    emit requestAction(REQ_CLOSE_GUI);
}
