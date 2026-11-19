// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#include "gifencoder.h"

#include <QByteArray>
#include <QFileInfo>

#include <algorithm>
#include <cstring>

namespace {

constexpr int kPaletteSize = 256;
constexpr int kColorCubeSize = 216;
constexpr int kGrayStart = kColorCubeSize;
constexpr int kGrayCount = kPaletteSize - kGrayStart;
constexpr int kClearCode = 256;
constexpr int kEndCode = 257;
constexpr int kMaxCode = 4096;
constexpr int kMinCodeSize = 8;

/**
 * Packs variable-width LZW codes into GIF sub-blocks (at most 255 bytes
 * each) and writes them to the output file.
 */
class BitWriter
{
public:
    explicit BitWriter(QFile& file)
      : m_file(file)
    {
        m_block.reserve(255);
    }

    void writeCode(int code, int size)
    {
        m_bitBuffer |= quint32(code) << m_bitCount;
        m_bitCount += size;
        while (m_bitCount >= 8) {
            m_block.append(char(m_bitBuffer & 0xFF));
            m_bitBuffer >>= 8;
            m_bitCount -= 8;
            if (m_block.size() == 255) {
                flushBlock();
            }
        }
    }

    void flush()
    {
        if (m_bitCount > 0) {
            m_block.append(char(m_bitBuffer & 0xFF));
            m_bitBuffer = 0;
            m_bitCount = 0;
        }
        flushBlock();
    }

    bool ok() const { return m_ok; }

private:
    void flushBlock()
    {
        if (m_block.isEmpty()) {
            return;
        }
        const char size = char(m_block.size());
        if (m_file.write(&size, 1) != 1 ||
            m_file.write(m_block.constData(), m_block.size()) !=
              m_block.size()) {
            m_ok = false;
        }
        m_block.clear();
    }

    QFile& m_file;
    QByteArray m_block;
    quint32 m_bitBuffer = 0;
    int m_bitCount = 0;
    bool m_ok = true;
};

int channelToLevel(int value)
{
    return (value * 5 + 127) / 255;
}

quint8 quantizeColor(int r, int g, int b)
{
    const int maximum = std::max({ r, g, b });
    const int minimum = std::min({ r, g, b });
    // Flat colors are mapped to the extra grayscale ramp, which avoids
    // banding on UI screenshots and text.
    if (maximum - minimum < 24) {
        const int luminance = (r * 299 + g * 587 + b * 114) / 1000;
        return quint8(kGrayStart +
                      (luminance * (kGrayCount - 1) + 127) / 255);
    }
    return quint8(channelToLevel(r) * 36 + channelToLevel(g) * 6 +
                  channelToLevel(b));
}

} // namespace

GifEncoder::GifEncoder(int width, int height, int loopCount)
  : m_width(width)
  , m_height(height)
  , m_loopCount(std::max(0, loopCount))
  , m_open(false)
  , m_colorLut(1 << 15)
  , m_lzwDictionary(1 << 20, 0xFFFF)
{
    buildColorLut();
}

GifEncoder::~GifEncoder()
{
    if (m_open) {
        close();
    }
}

void GifEncoder::buildColorLut()
{
    // The lookup table is indexed with 5 bits per channel, which makes the
    // per-pixel quantization a single array lookup.
    for (int r = 0; r < 32; ++r) {
        const int rv = r * 255 / 31;
        for (int g = 0; g < 32; ++g) {
            const int gv = g * 255 / 31;
            for (int b = 0; b < 32; ++b) {
                const int bv = b * 255 / 31;
                m_colorLut[(r << 10) | (g << 5) | b] =
                  quantizeColor(rv, gv, bv);
            }
        }
    }
}

quint8 GifEncoder::colorIndex(QRgb color) const
{
    const int index = ((qRed(color) >> 3) << 10) | ((qGreen(color) >> 3) << 5) |
                      (qBlue(color) >> 3);
    return m_colorLut.at(index);
}

void GifEncoder::putByte(quint8 value)
{
    const char byte = char(value);
    if (m_file.write(&byte, 1) != 1) {
        m_error = m_file.errorString();
    }
}

void GifEncoder::putU16(quint16 value)
{
    const char bytes[2] = { char(value & 0xFF), char((value >> 8) & 0xFF) };
    if (m_file.write(bytes, 2) != 2) {
        m_error = m_file.errorString();
    }
}

void GifEncoder::putBytes(const char* data, qint64 size)
{
    if (m_file.write(data, size) != size) {
        m_error = m_file.errorString();
    }
}

bool GifEncoder::open(const QString& filePath)
{
    if (m_open) {
        m_error = QStringLiteral("The GIF encoder is already open");
        return false;
    }
    if (m_width <= 0 || m_height <= 0 || m_width > 65535 || m_height > 65535) {
        m_error = QStringLiteral("Invalid GIF dimensions");
        return false;
    }
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_error = m_file.errorString();
        return false;
    }
    m_error.clear();
    m_open = true;
    if (!writeHeader()) {
        m_file.close();
        m_open = false;
        return false;
    }
    return true;
}

bool GifEncoder::writeHeader()
{
    putBytes("GIF89a", 6);

    putU16(quint16(m_width));
    putU16(quint16(m_height));
    // Global color table, 8-bit color resolution, 256 entries.
    putByte(0xF7);
    putByte(0x00); // background color index
    putByte(0x00); // pixel aspect ratio

    QByteArray palette;
    palette.reserve(kPaletteSize * 3);
    for (int i = 0; i < kColorCubeSize; ++i) {
        const int r = (i / 36) % 6;
        const int g = (i / 6) % 6;
        const int b = i % 6;
        palette.append(char(r * 51));
        palette.append(char(g * 51));
        palette.append(char(b * 51));
    }
    for (int i = 0; i < kGrayCount; ++i) {
        const char value = char(i * 255 / (kGrayCount - 1));
        palette.append(value);
        palette.append(value);
        palette.append(value);
    }
    putBytes(palette.constData(), palette.size());

    // Netscape application extension: loop count (0 = forever).
    putByte(0x21);
    putByte(0xFF);
    putByte(0x0B);
    putBytes("NETSCAPE2.0", 11);
    putByte(0x03);
    putByte(0x01);
    putU16(quint16(m_loopCount));
    putByte(0x00);

    return m_error.isEmpty();
}

bool GifEncoder::addFrame(const QImage& image, int delayCentiseconds)
{
    if (!m_open) {
        m_error = QStringLiteral("The GIF encoder is not open");
        return false;
    }
    if (image.isNull()) {
        m_error = QStringLiteral("Cannot add an empty frame");
        return false;
    }

    QImage frame = image;
    if (frame.size() != QSize(m_width, m_height)) {
        frame = frame.scaled(
          m_width, m_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    if (frame.format() != QImage::Format_RGB32) {
        frame = frame.convertToFormat(QImage::Format_RGB32);
    }

    if (!writeFrame(frame, std::max(0, delayCentiseconds))) {
        return false;
    }
    return m_error.isEmpty();
}

bool GifEncoder::writeFrame(const QImage& image, int delayCentiseconds)
{
    // Graphic control extension. Disposal method 1 keeps each full frame on
    // screen until the next one is drawn.
    putByte(0x21);
    putByte(0xF9);
    putByte(0x04);
    putByte(0x04);
    putU16(quint16(delayCentiseconds));
    putByte(0x00); // transparent color index (unused)
    putByte(0x00);

    // Image descriptor, full logical screen, no local color table.
    putByte(0x2C);
    putU16(0);
    putU16(0);
    putU16(quint16(m_width));
    putU16(quint16(m_height));
    putByte(0x00);

    QVector<quint8>& indices = m_indices;
    indices.resize(m_width * m_height);
    for (int y = 0; y < m_height; ++y) {
        const QRgb* line =
          reinterpret_cast<const QRgb*>(image.constScanLine(y));
        quint8* target = indices.data() + y * m_width;
        for (int x = 0; x < m_width; ++x) {
            target[x] = colorIndex(line[x]);
        }
    }

    if (!writeLzwData(indices)) {
        return false;
    }
    return m_error.isEmpty();
}

bool GifEncoder::writeLzwData(const QVector<quint8>& indices)
{
    putByte(kMinCodeSize);

    if (indices.isEmpty()) {
        putByte(0x00);
        return m_error.isEmpty();
    }

    BitWriter writer(m_file);
    int codeSize = kMinCodeSize + 1;
    int nextCode = kEndCode + 1;
    // A key is (prefix << 8) | suffix and fits in 20 bits.
    QVector<quint16>& dictionary = m_lzwDictionary;
    dictionary.fill(0xFFFF);

    writer.writeCode(kClearCode, codeSize);

    int prefix = indices.at(0);
    for (qsizetype i = 1; i < indices.size(); ++i) {
        const int suffix = indices.at(i);
        const quint32 key = (quint32(prefix) << 8) | quint32(suffix);
        const quint16 known = dictionary.at(key);
        if (known != 0xFFFF) {
            prefix = known;
            continue;
        }

        writer.writeCode(prefix, codeSize);
        if (nextCode < kMaxCode) {
            dictionary[key] = quint16(nextCode);
            ++nextCode;
            // GIF uses "early change": both sides widen the code as soon as
            // the next free entry no longer fits into the current width.
            if (nextCode > (1 << codeSize) && codeSize < 12) {
                ++codeSize;
            }
        } else {
            writer.writeCode(kClearCode, codeSize);
            dictionary.fill(0xFFFF);
            nextCode = kEndCode + 1;
            codeSize = kMinCodeSize + 1;
        }
        prefix = suffix;
    }

    writer.writeCode(prefix, codeSize);
    writer.writeCode(kEndCode, codeSize);
    writer.flush();
    putByte(0x00); // end of image data

    if (!writer.ok()) {
        m_error = QStringLiteral("Failed to write GIF image data");
        return false;
    }
    return m_error.isEmpty();
}

bool GifEncoder::close()
{
    if (!m_open) {
        return false;
    }
    putByte(0x3B); // GIF trailer
    m_file.flush();
    m_file.close();
    m_open = false;
    return m_error.isEmpty();
}
