#pragma once

#include <QPainter>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

// Thanks to Stackoverflow user "user898678": https://stackoverflow.com/a/10019508
// https://github.com/zdenop/qt-box-editor/blob/master/src/TessTools.cpp
// LICENSE: https://github.com/zdenop/qt-box-editor/blob/master/LICENSE
// modified version

class TesseractTool
{
public:
    static PIX* qImageToPIX(const QImage& qImage)
    {
        PIX* pixs;
        l_uint32* lines;

        QImage qImageCopy = qImage.copy();

        qImageCopy = qImageCopy.rgbSwapped();
        int width = qImageCopy.width();
        int height = qImageCopy.height();
        int depth = qImageCopy.depth();
        int wpl = qImageCopy.bytesPerLine() / 4;

        pixs = pixCreate(width, height, depth);
        pixSetWpl(pixs, wpl);
        pixSetColormap(pixs, NULL);
        l_uint32* datas = pixs->data;

        for (int y = 0; y < height; y++) {
            lines = datas + y * wpl;
            QByteArray a((const char*)qImageCopy.scanLine(y),
                         qImageCopy.bytesPerLine());
            for (int j = 0; j < a.size(); j++) {
                *((l_uint8*)lines + j) = a[j];
            }
        }
        return pixEndianByteSwapNew(pixs);
    }
};
