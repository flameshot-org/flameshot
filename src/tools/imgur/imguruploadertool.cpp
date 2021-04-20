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

ToolType ImgurUploaderTool::nameID() const
{
    return ToolType::IMGUR;
}

QString ImgurUploaderTool::description() const
{
    return tr("Upload the selection to Imgur");
}

QWidget* ImgurUploaderTool::widget()
{
    return new ImgurUploader(capture);
}

CaptureTool* ImgurUploaderTool::copy(QObject* parent)
{
    return new ImgurUploaderTool(parent);
}

void ImgurUploaderTool::pressed(const CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
