// Copyright 2017 Alejandro Sirgo Rica
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

#include "imguruploadertool.h"
#include <QPainter>

ImgurUploaderTool::ImgurUploaderTool(QObject *parent) : CaptureTool(parent) {

}

bool ImgurUploaderTool::isSelectable() {
    return false;
}

QString ImgurUploaderTool::getIconName() {
    return "cloud-upload.png";
}

QString ImgurUploaderTool::getName() {
    return tr("Image Uploader");
}

QString ImgurUploaderTool::getDescription() {
    return tr("Uploads the selection to Imgur");
}

CaptureTool::ToolWorkType ImgurUploaderTool::getToolType() {
    return TYPE_WORKER;
}

void ImgurUploaderTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color)
{
    Q_UNUSED(painter);
    Q_UNUSED(points);
    Q_UNUSED(color);
}

void ImgurUploaderTool::onPressed() {
    Q_EMIT requestAction(REQ_UPLOAD_TO_IMGUR);
}
