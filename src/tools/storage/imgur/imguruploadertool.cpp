// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "imguruploadertool.h"
#include "imguruploader.h"
#include <QPainter>

ImgurUploaderTool::ImgurUploaderTool(QObject* parent)
  : ImgUploaderTool(parent)
{}

QString ImgurUploaderTool::name() const
{
    return tr("Image Uploader");
}

QString ImgurUploaderTool::description() const
{
    return tr("Upload the selection to Imgur");
}

QWidget* ImgurUploaderTool::widget()
{
    ImgurUploader* p = new ImgurUploader(capture());
    p->upload();
    return p;
}

CaptureTool* ImgurUploaderTool::copy(QObject* parent)
{
    return new ImgurUploaderTool(parent);
}
