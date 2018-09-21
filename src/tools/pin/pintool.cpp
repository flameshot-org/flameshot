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

#include "pintool.h"
#include "src/tools/pin/pinwidget.h"

PinTool::PinTool(QObject *parent) : AbstractActionTool(parent) {

}

bool PinTool::closeOnButtonPressed() const {
    return true;
}

QIcon PinTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "pin.svg");
}
QString PinTool::name() const {
    return tr("Pin Tool");
}

QString PinTool::nameID() {
    return "";
}

QString PinTool::description() const {
    return tr("Pin image on the desktop");
}

QWidget* PinTool::widget() {
    PinWidget *w = new PinWidget(m_pixmap);
    const int &&m = w->margin();
    QRect adjusted_pos = m_geometry + QMargins(m, m, m, m);
    w->setGeometry(adjusted_pos);
    return w;
}

CaptureTool* PinTool::copy(QObject *parent) {
    return new PinTool(parent);
}

void PinTool::pressed(const CaptureContext &context) {
    emit requestAction(REQ_CAPTURE_DONE_OK);
    m_geometry = context.selection;
    m_geometry.setTopLeft(m_geometry.topLeft() + context.widgetOffset);
    m_pixmap = context.selectedScreenshotArea();
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);

}
