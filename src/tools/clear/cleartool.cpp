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

#include "cleartool.h"
#include <QPainter>

ClearTool::ClearTool(QObject *parent) : AbstractActionTool(parent) {

}

bool ClearTool::closeOnButtonPressed() const {
    return false;
}

QIcon ClearTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "clear.svg");
}
QString ClearTool::name() const {
    return tr("Clear");
}

QString ClearTool::nameID() {
    return QLatin1String("");
}

QString ClearTool::description() const {
    return tr("Clear all modifications");
}

CaptureTool* ClearTool::copy(QObject *parent) {
    return new ClearTool(parent);
}

void ClearTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
    emit requestAction(REQ_CLEAR_MODIFICATIONS);
}
