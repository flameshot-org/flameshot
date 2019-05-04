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

#include "barcodetool.h"
#include <QZXing.h>
#include "src/utils/systemnotification.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>

BarcodeTool::BarcodeTool(QObject *parent) : AbstractActionTool(parent) {

}

bool BarcodeTool::closeOnButtonPressed() const {
    return true;
}

QIcon BarcodeTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "qrcode-scan.svg");
}
QString BarcodeTool::name() const {
    return tr("Barcode");
}

QString BarcodeTool::nameID() {
    return QLatin1String("");
}

QString BarcodeTool::description() const {
    return tr("Get barcode of the selection into the clipboard");
}

CaptureTool* BarcodeTool::copy(QObject *parent) {
    return new BarcodeTool(parent);
}

void BarcodeTool::pressed(const CaptureContext &context) {
    emit requestAction(REQ_CAPTURE_DONE_OK);
    QZXing decoder;
    decoder.setDecoder(QZXing::DecoderFormat::DecoderFormat_QR_CODE
                       | QZXing::DecoderFormat::DecoderFormat_EAN_13);
    QString result = decoder.decodeImage(context.selectedScreenshotArea().toImage());
    if(result.length() > 0) {
        ScreenshotSaver().saveToClipboard(result);
    }else {
        SystemNotification().sendMessage(
                QObject::tr("Barcode not detected"));
    }
}
