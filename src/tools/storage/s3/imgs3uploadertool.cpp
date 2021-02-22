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

#include "imgs3uploadertool.h"
#include "imgs3uploader.h"
#include <QPainter>

ImgS3UploaderTool::ImgS3UploaderTool(QObject* parent)
  : ImgUploaderTool(parent)
{}

QString ImgS3UploaderTool::description() const
{
    return tr("Upload the selection to S3 bucket");
}

QWidget* ImgS3UploaderTool::widget()
{
    ImgS3Uploader* p = new ImgS3Uploader(capture());
    p->upload();
    return p;
}

CaptureTool* ImgS3UploaderTool::copy(QObject* parent)
{
    return new ImgS3UploaderTool(parent);
}
