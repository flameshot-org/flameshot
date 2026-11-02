// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 Flameshot Contributors

#pragma once

#ifdef ENABLE_QR_DECODER

#include <QList>
#include <QPolygonF>
#include <QString>

class QImage;
class QPixmap;

/**
 * @brief Result of a QR/barcode decode attempt.
 */
struct QrDecodeResult
{
    bool found = false;         ///< true if a barcode was successfully decoded
    QString content;            ///< decoded text content
    QPolygonF position;         ///< corner points of the barcode within the image
    QString format;             ///< barcode format name, e.g. "QR_CODE", "DATA_MATRIX"
};

/**
 * @brief Thread-safe utility class for decoding QR codes using ZXing-cpp.
 *
 * All methods are static and can safely be invoked from any thread,
 * including background threads via QtConcurrent::run().
 */
class QrDecoder
{
public:
    /**
     * @brief Decode a single QR/barcode from a QPixmap.
     * @param pixmap Source image to scan.
     * @return QrDecodeResult with found=true and content populated if a code was detected.
     */
    static QrDecodeResult decode(const QPixmap& pixmap);

    /**
     * @brief Decode a single QR/barcode from a QImage.
     * @param image Source image to scan.
     * @return QrDecodeResult with found=true and content populated if a code was detected.
     */
    static QrDecodeResult decode(const QImage& image);

    /**
     * @brief Decode all QR/barcodes found in a QPixmap.
     * @param pixmap Source image to scan.
     * @return List of all decoded results (may be empty).
     */
    static QList<QrDecodeResult> decodeAll(const QPixmap& pixmap);

private:
    QrDecoder() = delete;
};

#endif // ENABLE_QR_DECODER
