// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguruploadertool.h"
#include "imguruploader.h"
#include <QPainter>

ImgurUploaderTool::ImgurUploaderTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool ImgurUploaderTool::closeOnButtonPressed() const
{
    return true;
}

QIcon ImgurUploaderTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}

QString ImgurUploaderTool::name() const
{
    return tr("Image Uploader");
}

CaptureTool::Type ImgurUploaderTool::type() const
{
    return CaptureTool::TYPE_IMAGEUPLOADER;
}

QString ImgurUploaderTool::description() const
{
    return tr("Upload the selection to Imgur");
}

CaptureTool* ImgurUploaderTool::copy(QObject* parent)
{
    return new ImgurUploaderTool(parent);
}

void ImgurUploaderTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CAPTURE_DONE_OK);
    context.request.addTask(CaptureRequest::UPLOAD);
    emit requestAction(REQ_CLOSE_GUI);
}
