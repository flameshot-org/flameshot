// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QFile>
#include <QImage>
#include <QString>
#include <QVector>

/**
 * Minimal streaming GIF89a encoder.
 *
 * Frames are written to the file as soon as they are added, so a long
 * recording does not have to be kept in memory. The encoder uses a fixed
 * 256-color palette (a 6x6x6 color cube plus a grayscale ramp) and its own
 * LZW implementation, which keeps it independent from the optional Qt image
 * plugins.
 */
class GifEncoder
{
public:
    GifEncoder(int width, int height, int loopCount = 0);
    ~GifEncoder();

    bool open(const QString& filePath);
    bool addFrame(const QImage& image, int delayCentiseconds);
    bool close();

    bool isOpen() const { return m_open; }
    QString errorString() const { return m_error; }

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    GifEncoder(const GifEncoder&) = delete;
    GifEncoder& operator=(const GifEncoder&) = delete;

    bool writeHeader();
    bool writeFrame(const QImage& image, int delayCentiseconds);
    bool writeLzwData(const QVector<quint8>& indices);
    quint8 colorIndex(QRgb color) const;
    void buildColorLut();

    void putByte(quint8 value);
    void putU16(quint16 value);
    void putBytes(const char* data, qint64 size);

    int m_width;
    int m_height;
    int m_loopCount;

    QFile m_file;
    bool m_open;
    QString m_error;

    // 5 bits per channel -> palette index
    QVector<quint8> m_colorLut;
    // Per-frame scratch buffers, reused to avoid allocations per frame.
    QVector<quint8> m_indices;
    QVector<quint16> m_lzwDictionary;
};
