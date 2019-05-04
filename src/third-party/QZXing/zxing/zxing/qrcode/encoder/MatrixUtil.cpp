#include "MatrixUtil.h"
#include "MaskUtil.h"
#include <zxing/WriterException.h>
#include "QRCode.h"

namespace zxing {
namespace qrcode {

const int MatrixUtil::POSITION_DETECTION_PATTERN[7][7] =  {
    {1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1},
};

const int MatrixUtil::POSITION_ADJUSTMENT_PATTERN[5][5] = {
    {1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1},
    {1, 0, 1, 0, 1},
    {1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1},
};

// From Appendix E. Table 1, JIS0510X:2004 (p 71). The table was double-checked by komatsu.
const int MatrixUtil::POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[40][7] = {
    {-1, -1, -1, -1,  -1,  -1,  -1},  // Version 1
    { 6, 18, -1, -1,  -1,  -1,  -1},  // Version 2
    { 6, 22, -1, -1,  -1,  -1,  -1},  // Version 3
    { 6, 26, -1, -1,  -1,  -1,  -1},  // Version 4
    { 6, 30, -1, -1,  -1,  -1,  -1},  // Version 5
    { 6, 34, -1, -1,  -1,  -1,  -1},  // Version 6
    { 6, 22, 38, -1,  -1,  -1,  -1},  // Version 7
    { 6, 24, 42, -1,  -1,  -1,  -1},  // Version 8
    { 6, 26, 46, -1,  -1,  -1,  -1},  // Version 9
    { 6, 28, 50, -1,  -1,  -1,  -1},  // Version 10
    { 6, 30, 54, -1,  -1,  -1,  -1},  // Version 11
    { 6, 32, 58, -1,  -1,  -1,  -1},  // Version 12
    { 6, 34, 62, -1,  -1,  -1,  -1},  // Version 13
    { 6, 26, 46, 66,  -1,  -1,  -1},  // Version 14
    { 6, 26, 48, 70,  -1,  -1,  -1},  // Version 15
    { 6, 26, 50, 74,  -1,  -1,  -1},  // Version 16
    { 6, 30, 54, 78,  -1,  -1,  -1},  // Version 17
    { 6, 30, 56, 82,  -1,  -1,  -1},  // Version 18
    { 6, 30, 58, 86,  -1,  -1,  -1},  // Version 19
    { 6, 34, 62, 90,  -1,  -1,  -1},  // Version 20
    { 6, 28, 50, 72,  94,  -1,  -1},  // Version 21
    { 6, 26, 50, 74,  98,  -1,  -1},  // Version 22
    { 6, 30, 54, 78, 102,  -1,  -1},  // Version 23
    { 6, 28, 54, 80, 106,  -1,  -1},  // Version 24
    { 6, 32, 58, 84, 110,  -1,  -1},  // Version 25
    { 6, 30, 58, 86, 114,  -1,  -1},  // Version 26
    { 6, 34, 62, 90, 118,  -1,  -1},  // Version 27
    { 6, 26, 50, 74,  98, 122,  -1},  // Version 28
    { 6, 30, 54, 78, 102, 126,  -1},  // Version 29
    { 6, 26, 52, 78, 104, 130,  -1},  // Version 30
    { 6, 30, 56, 82, 108, 134,  -1},  // Version 31
    { 6, 34, 60, 86, 112, 138,  -1},  // Version 32
    { 6, 30, 58, 86, 114, 142,  -1},  // Version 33
    { 6, 34, 62, 90, 118, 146,  -1},  // Version 34
    { 6, 30, 54, 78, 102, 126, 150},  // Version 35
    { 6, 24, 50, 76, 102, 128, 154},  // Version 36
    { 6, 28, 54, 80, 106, 132, 158},  // Version 37
    { 6, 32, 58, 84, 110, 136, 162},  // Version 38
    { 6, 26, 54, 82, 110, 138, 166},  // Version 39
    { 6, 30, 58, 86, 114, 142, 170},  // Version 40
};

// Type info cells at the left top corner.
const int MatrixUtil::TYPE_INFO_COORDINATES[16][2] = {
    {8, 0},
    {8, 1},
    {8, 2},
    {8, 3},
    {8, 4},
    {8, 5},
    {8, 7},
    {8, 8},
    {7, 8},
    {5, 8},
    {4, 8},
    {3, 8},
    {2, 8},
    {1, 8},
    {0, 8},
};

// From Appendix D in JISX0510:2004 (p. 67)
const int MatrixUtil::VERSION_INFO_POLY = 0x1f25;  // 1 1111 0010 0101

// From Appendix C in JISX0510:2004 (p.65).
const int MatrixUtil::TYPE_INFO_POLY = 0x537;
const int MatrixUtil::TYPE_INFO_MASK_PATTERN = 0x5412;

void MatrixUtil::buildMatrix(const BitArray& dataBits,
                             const ErrorCorrectionLevel& ecLevel,
                             Version& version,
                             int maskPattern,
                             ByteMatrix& matrix)
{
    clearMatrix(matrix);
    embedBasicPatterns(version, matrix);
    // Type information appear with any version.
    embedTypeInfo(ecLevel, maskPattern, matrix);
    // Version info appear if version >= 7.
    maybeEmbedVersionInfo(version, matrix);
    // Data should be embedded at end.
    embedDataBits(dataBits, maskPattern, matrix);
}

void MatrixUtil::embedBasicPatterns(const Version& version, ByteMatrix& matrix)
{
    // Let's get started with embedding big squares at corners.
    embedPositionDetectionPatternsAndSeparators(matrix);
    // Then, embed the dark dot at the left bottom corner.
    embedDarkDotAtLeftBottomCorner(matrix);

    // Position adjustment patterns appear if version >= 2.
    maybeEmbedPositionAdjustmentPatterns(version, matrix);
    // Timing patterns should be embedded after position adj. patterns.
    embedTimingPatterns(matrix);
}

// Embed type information. On success, modify the matrix.
void MatrixUtil::embedTypeInfo(const ErrorCorrectionLevel& ecLevel, int maskPattern, ByteMatrix& matrix)
{
    BitArray typeInfoBits;
    makeTypeInfoBits(ecLevel, maskPattern, typeInfoBits);

    for (int i = 0; i < typeInfoBits.getSize(); ++i) {
        // Place bits in LSB to MSB order.  LSB (least significant bit) is the last value in
        // "typeInfoBits".
        bool bit = typeInfoBits.get(typeInfoBits.getSize() - 1 - i);

        // Type info bits at the left top corner. See 8.9 of JISX0510:2004 (p.46).
        int x1 = TYPE_INFO_COORDINATES[i][0];
        int y1 = TYPE_INFO_COORDINATES[i][1];
        matrix.set(size_t(x1), size_t(y1), bit);

        if (i < 8) {
            // Right top corner.
            int x2 = int(matrix.getWidth()) - i - 1;
            int y2 = 8;
            matrix.set(size_t(x2), size_t(y2), bit);
        } else {
            // Left bottom corner.
            int x2 = 8;
            int y2 = int(matrix.getHeight()) - 7 + (i - 8);
            matrix.set(size_t(x2), size_t(y2), bit);
        }
    }
}

void MatrixUtil::maybeEmbedVersionInfo(const Version& version, ByteMatrix& matrix)
{
    if (version.getVersionNumber() < 7) {  // Version info is necessary if version >= 7.
        return;  // Don't need version info.
    }
    BitArray versionInfoBits;
    makeVersionInfoBits(version, versionInfoBits);

    int bitIndex = 6 * 3 - 1;  // It will decrease from 17 to 0.
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 3; ++j) {
            // Place bits in LSB (least significant bit) to MSB order.
            boolean bit = versionInfoBits.get(bitIndex);
            bitIndex--;
            // Left bottom corner.
            matrix.set(size_t(i), size_t(int(matrix.getHeight()) - 11 + j), bit);
            // Right bottom corner.
            matrix.set(size_t(int(matrix.getHeight()) - 11 + j), size_t(i), bit);
        }
    }
}

void MatrixUtil::embedDataBits(const BitArray& dataBits, int maskPattern, ByteMatrix& matrix)
{
    int bitIndex = 0;
    int direction = -1;
    // Start from the right bottom cell.
    int x = int(matrix.getWidth()) - 1;
    int y = int(matrix.getHeight()) - 1;
    while (x > 0) {
        // Skip the vertical timing pattern.
        if (x == 6) {
            x -= 1;
        }
        while (y >= 0 && y < int(matrix.getHeight())) {
            for (int i = 0; i < 2; ++i) {
                int xx = x - i;
                // Skip the cell if it's not empty.
                if (!isEmpty(matrix.get(size_t(xx), size_t(y)))) {
                    continue;
                }
                boolean bit;
                if (bitIndex < dataBits.getSize()) {
                    bit = dataBits.get(bitIndex);
                    ++bitIndex;
                } else {
                    // Padding bit. If there is no bit left, we'll fill the left cells with 0, as described
                    // in 8.4.9 of JISX0510:2004 (p. 24).
                    bit = false;
                }

                // Skip masking if mask_pattern is -1.
                if (maskPattern != -1 && MaskUtil::getDataMaskBit(maskPattern, xx, y)) {
                    bit = !bit;
                }
                matrix.set(size_t(xx), size_t(y), bit);
            }
            y += direction;
        }
        direction = -direction;  // Reverse the direction.
        y += direction;
        x -= 2;  // Move to the left.
    }
    // All bits should be consumed.
    if (bitIndex != dataBits.getSize()) {
        throw zxing::WriterException("Not all bits consumed");
    }
}

int MatrixUtil::findMSBSet(int value)
{
    int sizeOfTypeBits = (sizeof(int)*8);
    int sample = ( value < 0 ) ? 0 : value;
    int leadingZeros = ( value < 0 ) ? 0 : sizeOfTypeBits;

    while(sample) {
        sample >>= 1;
        --leadingZeros;
    }

    return sizeOfTypeBits - leadingZeros;
}

int MatrixUtil::calculateBCHCode(int value, int poly)
{
    // If poly is "1 1111 0010 0101" (version info poly), msbSetInPoly is 13. We'll subtract 1
    // from 13 to make it 12.
    int msbSetInPoly = findMSBSet(poly);
    value <<= msbSetInPoly - 1;
    // Do the division business using exclusive-or operations.
    while (findMSBSet(value) >= msbSetInPoly) {
        value ^= poly << (findMSBSet(value) - msbSetInPoly);
    }
    // Now the "value" is the remainder (i.e. the BCH code)
    return value;
}

// Make bit vector of type information. On success, store the result in "bits" and return true.
// Encode error correction level and mask pattern. See 8.9 of
// JISX0510:2004 (p.45) for details.
void MatrixUtil::makeTypeInfoBits(const ErrorCorrectionLevel& ecLevel, int maskPattern, BitArray& bits)
{
    if (!QRCode::isValidMaskPattern(maskPattern)) {
        throw WriterException("Invalid mask pattern");
    }
    int typeInfo = (ecLevel.bits() << 3) | maskPattern;
    bits.appendBits(typeInfo, 5);

    int bchCode = calculateBCHCode(typeInfo, TYPE_INFO_POLY);
    bits.appendBits(bchCode, 10);

    BitArray maskBits;
    maskBits.appendBits(TYPE_INFO_MASK_PATTERN, 15);
    bits.xor_(maskBits);

    if (bits.getSize() != 15) {  // Just in case.
        throw WriterException("makeTypeInfoBits() failed, should not happen");
    }
}

// Make bit vector of version information. On success, store the result in "bits" and return true.
// See 8.10 of JISX0510:2004 (p.45) for details.
void MatrixUtil::makeVersionInfoBits(const Version& version, BitArray& bits)
{
    bits.appendBits(version.getVersionNumber(), 6);
    int bchCode = calculateBCHCode(version.getVersionNumber(), VERSION_INFO_POLY);
    bits.appendBits(bchCode, 12);

    if (bits.getSize() != 18) {  // Just in case.
        throw WriterException("makeVersionInfoBits() failed, should not happen");
    }
}

void MatrixUtil::embedTimingPatterns(ByteMatrix& matrix)
{
    // -8 is for skipping position detection patterns (size 7), and two horizontal/vertical
    // separation patterns (size 1). Thus, 8 = 7 + 1.
    for (size_t i = 8; i < matrix.getWidth() - 8; ++i) {
        int bit = (i + 1) % 2;
        // Horizontal line.
        if (isEmpty(matrix.get(i, 6))) {
            matrix.set(i, 6, zxing::byte(bit));
        }
        // Vertical line.
        if (isEmpty(matrix.get(6, i))) {
            matrix.set(6, i, zxing::byte(bit));
        }
    }
}

void MatrixUtil::embedDarkDotAtLeftBottomCorner(ByteMatrix& matrix)
{
    if (matrix.get(8, matrix.getHeight() - 8) == 0) {
        throw WriterException();
    }
    matrix.set(8, matrix.getHeight() - 8, zxing::byte(1));
}

void MatrixUtil::embedHorizontalSeparationPattern(int xStart,
                                                  int yStart,
                                                  ByteMatrix& matrix)
{
    for (int x = 0; x < 8; ++x) {
        if (!isEmpty(matrix.get(size_t(xStart + x), size_t(yStart)))) {
            throw WriterException();
        }
        matrix.set(size_t(xStart + x), size_t(yStart), zxing::byte(0));
    }
}

void MatrixUtil::embedVerticalSeparationPattern(int xStart,
                                                int yStart,
                                                ByteMatrix& matrix)
{
    for (int y = 0; y < 7; ++y) {
        if (!isEmpty(matrix.get(size_t(xStart), size_t(yStart + y)))) {
            throw WriterException();
        }
        matrix.set(size_t(xStart), size_t(yStart + y), zxing::byte(0));
    }
}

void MatrixUtil::embedPositionAdjustmentPattern(int xStart, int yStart, ByteMatrix& matrix)
{
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            matrix.set(size_t(xStart + x), size_t(yStart + y), zxing::byte(POSITION_ADJUSTMENT_PATTERN[y][x]));
        }
    }
}

void MatrixUtil::embedPositionDetectionPattern(int xStart, int yStart, ByteMatrix& matrix)
{
    for (int y = 0; y < 7; ++y) {
        for (int x = 0; x < 7; ++x) {
            matrix.set(size_t(xStart + x), size_t(yStart + y), zxing::byte(POSITION_DETECTION_PATTERN[y][x]));
        }
    }
}

void MatrixUtil::embedPositionDetectionPatternsAndSeparators(ByteMatrix& matrix)
{
    // Embed three big squares at corners.
    int pdpWidth = 7;//need to change this, old version: POSITION_DETECTION_PATTERN[0].length;
    // Left top corner.
    embedPositionDetectionPattern(0, 0, matrix);
    // Right top corner.
    embedPositionDetectionPattern(int(matrix.getWidth()) - pdpWidth, 0, matrix);
    // Left bottom corner.
    embedPositionDetectionPattern(0, int(matrix.getWidth()) - pdpWidth, matrix);

    // Embed horizontal separation patterns around the squares.
    int hspWidth = 8;
    // Left top corner.
    embedHorizontalSeparationPattern(0, hspWidth - 1, matrix);
    // Right top corner.
    embedHorizontalSeparationPattern(int(matrix.getWidth()) - hspWidth,
                                     hspWidth - 1, matrix);
    // Left bottom corner.
    embedHorizontalSeparationPattern(0, int(matrix.getWidth()) - hspWidth, matrix);

    // Embed vertical separation patterns around the squares.
    int vspSize = 7;
    // Left top corner.
    embedVerticalSeparationPattern(vspSize, 0, matrix);
    // Right top corner.
    embedVerticalSeparationPattern(int(matrix.getHeight()) - vspSize - 1, 0, matrix);
    // Left bottom corner.
    embedVerticalSeparationPattern(vspSize, int(matrix.getHeight()) - vspSize,
                                   matrix);
}

void MatrixUtil::maybeEmbedPositionAdjustmentPatterns(const Version& version, ByteMatrix& matrix)
{
    if (version.getVersionNumber() < 2) {  // The patterns appear if version >= 2
        return;
    }
    int index = version.getVersionNumber() - 1;
    const int *coordinates = POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[index];
    int numCoordinates = 7; //POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[index].length; //need to change the constant 7

    for (int i = 0; i < numCoordinates; i++) {
        int y = coordinates[i];
        if(y < 0)
            continue;

        for (int j = 0; j < numCoordinates; j++) {
            int x = coordinates[j];
            if (x < 0)
                continue;

            // If the cell is unset, we embed the position adjustment pattern here.
            if (isEmpty(matrix.get(size_t(x), size_t(y)))) {
                // -2 is necessary since the x/y coordinates point to the center of the pattern, not the
                // left top corner.
                embedPositionAdjustmentPattern(x - 2, y - 2, matrix);
            }
        }
    }
}

}
}
