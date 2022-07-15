// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguploadertool.h"

ImgUploaderTool::ImgUploaderTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool ImgUploaderTool::closeOnButtonPressed() const
{
    return true;
}

QIcon ImgUploaderTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}

QString ImgUploaderTool::name() const
{
    return tr("Image Uploader");
}

CaptureTool::Type ImgUploaderTool::type() const
{
    return CaptureTool::TYPE_IMAGEUPLOADER;
}

QString ImgUploaderTool::description() const
{
    return tr("Upload the selection");
}

CaptureTool* ImgUploaderTool::copy(QObject* parent)
{
    return new ImgUploaderTool(parent);
}

void ImgUploaderTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    context.request.addTask(CaptureRequest::UPLOAD);
    emit requestAction(REQ_CLOSE_GUI);
}
