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

#include "up1uploadertool.h"
#include "up1uploader.h"
#include <QPainter>

Up1UploaderTool::Up1UploaderTool(QObject *parent) : AbstractActionTool(parent) {

}

bool Up1UploaderTool::closeOnButtonPressed() const {
    return true;
}

QIcon Up1UploaderTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}
QString Up1UploaderTool::name() const {
    return tr("Image Uploader");
}

QString Up1UploaderTool::nameID() {
    return QLatin1String("");
}

QString Up1UploaderTool::description() const {
    return tr("Upload the selection to Up1");
}

QWidget* Up1UploaderTool::widget() {
    return new Up1Uploader(capture);
}

CaptureTool* Up1UploaderTool::copy(QObject *parent) {
    return new Up1UploaderTool(parent);
}

void Up1UploaderTool::pressed(const CaptureContext &context) {
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
