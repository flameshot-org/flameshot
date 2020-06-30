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
#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QSettings>


ImgS3UploaderTool::ImgS3UploaderTool(QObject *parent) : AbstractActionTool(parent) {
    QSettings *pSettings = new QSettings(QDir::currentPath() + "/config.ini", QSettings::IniFormat);
    pSettings->beginGroup("S3");
    m_s3CredsUrl = pSettings->value("S3_CREDS_URL").toString();
    m_s3XApiKey = pSettings->value("S3_X_API_KEY").toString();
}

bool ImgS3UploaderTool::closeOnButtonPressed() const {
    return true;
}

QIcon ImgS3UploaderTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}
QString ImgS3UploaderTool::name() const {
    return tr("Image Uploader");
}

QString ImgS3UploaderTool::nameID() {
    return QLatin1String("");
}

QString ImgS3UploaderTool::description() const {
    return tr("Upload the selection to ImgS3");
}

QWidget* ImgS3UploaderTool::widget() {
    return new ImgS3Uploader(capture, m_s3CredsUrl, m_s3XApiKey);
}

CaptureTool* ImgS3UploaderTool::copy(QObject *parent) {
    return new ImgS3UploaderTool(parent);
}

void ImgS3UploaderTool::pressed(const CaptureContext &context) {
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
