// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Flameshot Contributors

#include "qrdecoder.h"

#ifdef ENABLE_QR_DECODER

#include <QImage>
#include <QPixmap>
#include <QPointF>

// ZXing-cpp headers
#include <ReadBarcode.h>
#include <ReaderOptions.h>
#include <ImageView.h>
#include <BarcodeFormat.h>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

/**
 * Convert a QImage into a ZXing::ImageView. The QImage must stay alive for
 * the lifetime of the returned ImageView.
 */
static ZXing::ImageView toImageView(const QImage& img)
{
    // ZXing works best with RGB or Lum formats.
    // Qt's Format_RGB32 stores pixels as 0xffRRGGBB (4 bytes, blue at index 0
    // in little-endian memory), which corresponds to BGRX / XRGB depending on
    // endianness. The safest choice is to convert to Format_RGB888 (3-byte RGB)
    // and use ZXing::ImageFormat::RGB.
    return ZXing::ImageView(img.bits(),
                            img.width(),
                            img.height(),
                            ZXing::ImageFormat::RGB,
                            static_cast<int>(img.bytesPerLine()),
                            3 /*pixStride*/);
}

/**
 * Convert ZXing::Position (four corner points) into a QPolygonF.
 */
static QPolygonF toPolygon(const ZXing::Position& pos)
{
    QPolygonF poly;
    for (const auto& p : pos) {
        poly << QPointF(p.x, p.y);
    }
    return poly;
}

// ---------------------------------------------------------------------------
// QrDecoder implementation
// ---------------------------------------------------------------------------

QrDecodeResult QrDecoder::decode(const QImage& image)
{
    if (image.isNull()) {
        return {};
    }

    // Ensure the image is in a contiguous RGB888 format that ZXing can handle.
    QImage rgb = image.convertToFormat(QImage::Format_RGB888);

    ZXing::ReaderOptions opts;
    // Restrict to QR code only for performance; can be expanded later.
    opts.setFormats(ZXing::BarcodeFormat::QRCode);
    opts.setTryHarder(true);
    opts.setTryRotate(true);

    ZXing::Result result = ZXing::ReadBarcode(toImageView(rgb), opts);

    if (!result.isValid()) {
        return {};
    }

    QrDecodeResult r;
    r.found   = true;
    r.content = QString::fromStdString(result.text());
    r.format  = QString::fromStdString(ZXing::ToString(result.format()));
    r.position = toPolygon(result.position());
    return r;
}

QrDecodeResult QrDecoder::decode(const QPixmap& pixmap)
{
    if (pixmap.isNull()) {
        return {};
    }
    return decode(pixmap.toImage());
}

QList<QrDecodeResult> QrDecoder::decodeAll(const QPixmap& pixmap)
{
    QList<QrDecodeResult> results;

    if (pixmap.isNull()) {
        return results;
    }

    QImage rgb = pixmap.toImage().convertToFormat(QImage::Format_RGB888);

    ZXing::ReaderOptions opts;
    opts.setFormats(ZXing::BarcodeFormat::QRCode);
    opts.setTryHarder(true);
    opts.setTryRotate(true);
    opts.setMaxNumberOfSymbols(10);

    ZXing::Results zResults = ZXing::ReadBarcodes(toImageView(rgb), opts);

    for (const auto& res : zResults) {
        if (res.isValid()) {
            QrDecodeResult r;
            r.found    = true;
            r.content  = QString::fromStdString(res.text());
            r.format   = QString::fromStdString(ZXing::ToString(res.format()));
            r.position = toPolygon(res.position());
            results.append(r);
        }
    }

    return results;
}

#endif // ENABLE_QR_DECODER
