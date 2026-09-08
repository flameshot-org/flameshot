// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "ocrtool.h"
#include "core/flameshot.h"
#include "core/flameshotdaemon.h"
#include "ocrwidget.h"
#include "tools/capturecontext.h"
#include "utils/abstractlogger.h"
#include "utils/colorutils.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

OcrTool::OcrTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool OcrTool::closeOnButtonPressed() const
{
    return true;
}

QIcon OcrTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    if (ColorUtils::colorIsDark(background)) {
        return QIcon(QStringLiteral(":/ocrplugin/icons/white_ocr.svg"));
    } else {
        return QIcon(QStringLiteral(":/ocrplugin/icons/black_ocr.svg"));
    }
}

QString OcrTool::name() const
{
    return tr("Text Recognition (OCR)");
}

CaptureTool::Type OcrTool::type() const
{
    return CaptureTool::NONE;
}

QString OcrTool::description() const
{
    return tr("Extract text from the selected region using Tesseract OCR");
}

CaptureTool* OcrTool::copy(QObject* parent)
{
    return new OcrTool(parent);
}

void OcrTool::pressed(CaptureContext& context)
{
    QPixmap capture = context.selectedScreenshotArea();
    QRect selection = context.selection;
    if (selection.isNull() || selection.isEmpty()) {
        selection = context.screenshot.rect();
    }

    if (capture.isNull() || capture.width() < 4 || capture.height() < 4) {
        emit requestAction(REQ_CLOSE_GUI);
        return;
    }

    // 1. High-quality smooth scaling (2.5x - 3.0x) to optimize character
    // resolution for Tesseract LSTM
    QImage srcImg = capture.toImage();
    const int origWidth = srcImg.width();
    const int origHeight = srcImg.height();

    qreal scaleFactor = 2.5;
    if (origWidth < 350 || origHeight < 350) {
        scaleFactor = 3.0;
    } else if (origWidth > 1600 || origHeight > 1600) {
        scaleFactor = 1.8;
    }

    const int targetW = qMax(1, qRound(origWidth * scaleFactor));
    const int targetH = qMax(1, qRound(origHeight * scaleFactor));
    QImage img = srcImg.scaled(
      targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    img = img.convertToFormat(QImage::Format_Grayscale8);

    const int width = img.width();
    const int height = img.height();

    // 2. Measure luminance distribution and detect background polarity
    long long globalSum = 0;
    int minVal = 255;
    int maxVal = 0;
    for (int y = 0; y < height; ++y) {
        const uchar* line = img.constScanLine(y);
        for (int x = 0; x < width; ++x) {
            const uchar val = line[x];
            globalSum += val;
            if (val < minVal)
                minVal = val;
            if (val > maxVal)
                maxVal = val;
        }
    }
    const bool isLightBg =
      (globalSum / (static_cast<long long>(width) * height)) > 127;

    // 3. Normalize grayscale contrast and invert dark mode while preserving
    // anti-aliased subpixel edges
    QImage procImg(width, height, QImage::Format_Grayscale8);
    const int range = qMax(1, maxVal - minVal);

    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = img.constScanLine(y);
        uchar* dstLine = procImg.scanLine(y);
        for (int x = 0; x < width; ++x) {
            const int v = srcLine[x];
            int norm = (v - minVal) * 255 / range;
            norm = qBound(0, norm, 255);
            if (!isLightBg) {
                dstLine[x] = static_cast<uchar>(255 - norm);
            } else {
                dstLine[x] = static_cast<uchar>(norm);
            }
        }
    }

    // 4. Fast, highly accurate primary multi-language loader (<0.2s execution)
    QStringList targetLangs = { "eng", "ben", "hin", "osd" };
    QStringList activeLangs;
    const QStringList searchDirs = {
        QStringLiteral("/usr/share/tesseract-ocr/5/tessdata/"),
        QStringLiteral("/usr/share/tesseract-ocr/4.00/tessdata/"),
        QStringLiteral("/usr/share/tessdata/")
    };
    for (const QString& l : targetLangs) {
        for (const QString& dir : searchDirs) {
            if (QFile::exists(dir + l + QStringLiteral(".traineddata"))) {
                if (!activeLangs.contains(l)) {
                    activeLangs.append(l);
                }
                break;
            }
        }
    }
    if (activeLangs.isEmpty()) {
        activeLangs.append(QStringLiteral("eng"));
    }
    QByteArray langBytes = activeLangs.join(QStringLiteral("+")).toUtf8();

    auto* api = new tesseract::TessBaseAPI();
    int initStatus = api->Init(nullptr, langBytes.constData());
    if (initStatus != 0) {
        initStatus = api->Init(nullptr, "eng");
    }

    QVector<QRect> lineBoxes;
    QStringList lines;
    QVector<QRect> wordBoxes;
    QStringList words;

    if (initStatus == 0) {
        api->SetVariable("load_system_dawg", "0");
        api->SetVariable("load_freq_dawg", "0");
        api->SetVariable("user_defined_dpi", "300");
        api->SetVariable("preserve_interword_spaces", "1");
        api->SetPageSegMode(tesseract::PSM_AUTO);

        api->SetImage(procImg.bits(),
                      procImg.width(),
                      procImg.height(),
                      1,
                      procImg.bytesPerLine());
        api->Recognize(0);

        // Helper to detect colored UI emojis
        auto detectEmojiInBox = [&](const QRect& bBox) -> QString {
            QRect r = bBox.intersected(srcImg.rect());
            if (r.width() < 8 || r.height() < 8)
                return QString();
            qreal aspect = static_cast<qreal>(r.width()) / qMax(1, r.height());
            if (aspect > 1.45 || aspect < 0.65)
                return QString();

            int totalColored = 0, redCount = 0, yellowCount = 0, greenCount = 0;
            for (int y = r.y(); y <= r.bottom(); ++y) {
                const QRgb* line =
                  reinterpret_cast<const QRgb*>(srcImg.constScanLine(y));
                for (int x = r.x(); x <= r.right(); ++x) {
                    QColor c(line[x]);
                    if (c.hsvSaturation() >= 85 && c.value() >= 55) {
                        totalColored++;
                        int h = c.hsvHue();
                        if (h >= 330 || h <= 18) {
                            redCount++;
                        } else if (h >= 26 && h <= 62) {
                            yellowCount++;
                        } else if (h >= 80 && h <= 155) {
                            greenCount++;
                        }
                    }
                }
            }
            int area = r.width() * r.height();
            if (totalColored >= 18 && totalColored >= area * 0.12) {
                if (redCount >= totalColored * 0.45)
                    return QStringLiteral("❤️");
                if (yellowCount >= totalColored * 0.45)
                    return QStringLiteral("👍");
                if (greenCount >= totalColored * 0.45)
                    return QStringLiteral("✅");
            }
            return QString();
        };

        tesseract::ResultIterator* ri = api->GetIterator();
        if (ri) {
            do {
                QVector<QRect> curLineWordBoxes;
                QStringList curLineWords;
                int lx1 = 0, ly1 = 0, lx2 = 0, ly2 = 0;
                ri->BoundingBox(
                  tesseract::RIL_TEXTLINE, &lx1, &ly1, &lx2, &ly2);

                do {
                    const char* wText = ri->GetUTF8Text(tesseract::RIL_WORD);
                    if (wText) {
                        QString wStr = QString::fromUtf8(wText).trimmed();
                        delete[] wText;
                        wStr.replace(QRegularExpression(QStringLiteral(
                                       "^[@©®¢¥~|](?=[A-Za-z\\p{L}])")),
                                     QString());
                        wStr.replace(
                          QRegularExpression(QStringLiteral("@(?=[0-9])")),
                          QStringLiteral("0"));
                        wStr.replace(
                          QRegularExpression(QStringLiteral("(?<=[0-9])@")),
                          QStringLiteral("0"));
                        wStr = wStr.trimmed();

                        int x1, y1, x2, y2;
                        ri->BoundingBox(
                          tesseract::RIL_WORD, &x1, &y1, &x2, &y2);
                        int bx = qRound(x1 / scaleFactor);
                        int by = qRound(y1 / scaleFactor);
                        int bw = qMax(1, qRound((x2 - x1) / scaleFactor));
                        int bh = qMax(1, qRound((y2 - y1) / scaleFactor));
                        QRect wRect(bx, by, bw, bh);

                        QString emoji = detectEmojiInBox(wRect);
                        if (!emoji.isEmpty()) {
                            if (wStr == QStringLiteral("Y") ||
                                wStr == QStringLiteral("の") ||
                                wStr == QStringLiteral("ول") ||
                                wStr == QStringLiteral("J") ||
                                wStr == QStringLiteral("4") ||
                                wStr == QStringLiteral("€") ||
                                wStr == QStringLiteral("å")) {
                                wStr = emoji;
                            }
                        }

                        if (!wStr.isEmpty()) {
                            wordBoxes.append(wRect);
                            words.append(wStr);
                            curLineWordBoxes.append(wRect);
                            curLineWords.append(wStr);
                        }
                    }
                } while (!ri->IsAtFinalElement(tesseract::RIL_TEXTLINE,
                                               tesseract::RIL_WORD) &&
                         ri->Next(tesseract::RIL_WORD));

                if (!curLineWords.isEmpty()) {
                    int bx = qRound(lx1 / scaleFactor);
                    int by = qRound(ly1 / scaleFactor);
                    int bw = qMax(1, qRound((lx2 - lx1) / scaleFactor));
                    int bh = qMax(1, qRound((ly2 - ly1) / scaleFactor));
                    lineBoxes.append(QRect(bx, by, bw, bh));

                    QString lStr = curLineWords.join(QStringLiteral(" "));
                    lStr.replace(
                      QRegularExpression(QStringLiteral(
                        "(^|\\n)[ওe◦o°*+•»›-]\\s+(?=[A-Za-z0-9\\p{L}#])")),
                      QStringLiteral("\\1• "));
                    lines.append(lStr);
                }
            } while (ri->Next(tesseract::RIL_TEXTLINE));
        }

        api->End();
        delete api;
    }

    QString result = lines.join(QStringLiteral("\n"));
    if (!result.isEmpty()) {
        QApplication::clipboard()->setText(result);
        AbstractLogger::info()
          << QObject::tr("OCR: Text extracted and copied to clipboard.");
    } else {
        AbstractLogger::info()
          << QObject::tr("OCR: No text recognized in selection.");
    }

    // Keep the application running for the OCR interactive inspector window
    Flameshot::instance()->setExternalWidget(true);

    // Launch OCR Interactive Canvas Window
    auto* widget = new OcrWidget(
      capture, selection, result, wordBoxes, words, lineBoxes, lines, nullptr);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(widget, &QWidget::destroyed, []() {
        Flameshot::instance()->setExternalWidget(false);
        if (FlameshotDaemon::instance() == nullptr) {
            qApp->quit();
        }
    });
    widget->show();
    widget->raise();
    widget->activateWindow();

    emit requestAction(REQ_CLEAR_SELECTION);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
