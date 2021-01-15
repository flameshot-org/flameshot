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

#include "ocrtool.h"
#include "tesseract_tool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>
#include <tesseract/basetesseractApi.h>
#include <leptonica/allheaders.h>

OcrTool::OcrTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool OcrTool::closeOnButtonPressed() const
{
    return true;
}

QIcon OcrTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "ocr.svg");
}
QString OcrTool::name() const
{
    return "OCR";
}

ToolType OcrTool::nameID() const
{
    return ToolType::OCR;
}

QString OcrTool::description() const
{
    return "Copy text in Capture to Clipboard";
}

CaptureTool* OcrTool::copy(QObject* parent)
{
    return new OcrTool(parent);
}

void OcrTool::pressed(const CaptureContext& context)
{
    char *outText;

    tesseract::TessBaseAPI *tesseractApi = new tesseract::TessBaseAPI();
    
    // TODO: tesseract language configs?
    if (tesseractApi->Init(NULL, "eng")) {
        // TODO: error system notification?
        return;
    }

    Pix *image = TesseractTool::qImage2PIX(context.selectedScreenshotArea().toImage());
    tesseractApi->SetImage(image);

    outText = tesseractApi->GetUTF8Text();
    printf("OCR output:\n%s", outText);

    const QString qString = outText;

    ScreenshotSaver().saveToClipboard(qString);

    tesseractApi->End();
    delete tesseractApi;
    delete [] outText;
}