#ifndef MATRIXUTIL_H
#define MATRIXUTIL_H

//package com.google.zxing.qrcode.encoder;
//import com.google.zxing.WriterException;
//import com.google.zxing.common.BitArray;
//import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;
//import com.google.zxing.qrcode.decoder.Version;

#include "ByteMatrix.h"
#include <zxing/common/BitArray.h>
#include <zxing/qrcode/ErrorCorrectionLevel.h>
#include <zxing/qrcode/Version.h>

namespace zxing {
namespace qrcode {

class MatrixUtil {

private:
    MatrixUtil() {}

    static const int POSITION_DETECTION_PATTERN[7][7];
    static const int POSITION_ADJUSTMENT_PATTERN[5][5];
    static const int POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[40][7];
    static const int TYPE_INFO_COORDINATES[16][2];
    static const int VERSION_INFO_POLY;
    static const int TYPE_INFO_POLY;
    static const int TYPE_INFO_MASK_PATTERN;

private:
    // Check if "value" is empty.
    static bool isEmpty(int value) { return value == 255; }
    //static bool isEmpty(int value) { return value == -1; }

    static void embedTimingPatterns(ByteMatrix& matrix);

    // Embed the lonely dark dot at left bottom corner. JISX0510:2004 (p.46)
    static void embedDarkDotAtLeftBottomCorner(ByteMatrix& matrix);

    static void embedHorizontalSeparationPattern(int xStart,
                                                 int yStart,
                                                 ByteMatrix& matrix);

    static void embedVerticalSeparationPattern(int xStart,
                                               int yStart,
                                               ByteMatrix& matrix);

    // Note that we cannot unify the function with embedPositionDetectionPattern() despite they are
    // almost identical, since we cannot write a function that takes 2D arrays in different sizes in
    // C/C++. We should live with the fact.
    static void embedPositionAdjustmentPattern(int xStart, int yStart, ByteMatrix& matrix);

    static void embedPositionDetectionPattern(int xStart, int yStart, ByteMatrix& matrix);

    // Embed position detection patterns and surrounding vertical/horizontal separators.
    static void embedPositionDetectionPatternsAndSeparators(ByteMatrix& matrix);

    // Embed position adjustment patterns if need be.
    static void maybeEmbedPositionAdjustmentPatterns(const Version& version, ByteMatrix& matrix);

public:
    // Set all cells to -1.  -1 means that the cell is empty (not set yet).
    static void clearMatrix(ByteMatrix& matrix) {
        matrix.clear((zxing::byte) -1);
    }

    // Embed basic patterns. On success, modify the matrix and return true.
    // The basic patterns are:
    // - Position detection patterns
    // - Timing patterns
    // - Dark dot at the left bottom corner
    // - Position adjustment patterns, if need be
    static void embedBasicPatterns(const Version& version, ByteMatrix& matrix);

    // Embed type information. On success, modify the matrix.
    static void embedTypeInfo(const ErrorCorrectionLevel& ecLevel, int maskPattern, ByteMatrix& matrix);

    // Embed version information if need be. On success, modify the matrix and return true.
    // See 8.10 of JISX0510:2004 (p.47) for how to embed version information.
    static void maybeEmbedVersionInfo(const Version& version, ByteMatrix& matrix);

    // Embed "dataBits" using "getMaskPattern". On success, modify the matrix and return true.
    // For debugging purposes, it skips masking process if "getMaskPattern" is -1.
    // See 8.7 of JISX0510:2004 (p.38) for how to embed data bits.
    static void embedDataBits(const BitArray& dataBits, int maskPattern, ByteMatrix& matrix);

    // Return the position of the most significant bit set (to one) in the "value". The most
    // significant bit is position 32. If there is no bit set, return 0. Examples:
    // - findMSBSet(0) => 0
    // - findMSBSet(1) => 1
    // - findMSBSet(255) => 8
    static int findMSBSet(int value);

    // Calculate BCH (Bose-Chaudhuri-Hocquenghem) code for "value" using polynomial "poly". The BCH
    // code is used for encoding type information and version information.
    // Example: Calculation of version information of 7.
    // f(x) is created from 7.
    //   - 7 = 000111 in 6 bits
    //   - f(x) = x^2 + x^1 + x^0
    // g(x) is given by the standard (p. 67)
    //   - g(x) = x^12 + x^11 + x^10 + x^9 + x^8 + x^5 + x^2 + 1
    // Multiply f(x) by x^(18 - 6)
    //   - f'(x) = f(x) * x^(18 - 6)
    //   - f'(x) = x^14 + x^13 + x^12
    // Calculate the remainder of f'(x) / g(x)
    //         x^2
    //         __________________________________________________
    //   g(x) )x^14 + x^13 + x^12
    //         x^14 + x^13 + x^12 + x^11 + x^10 + x^7 + x^4 + x^2
    //         --------------------------------------------------
    //                              x^11 + x^10 + x^7 + x^4 + x^2
    //
    // The remainder is x^11 + x^10 + x^7 + x^4 + x^2
    // Encode it in binary: 110010010100
    // The return value is 0xc94 (1100 1001 0100)
    //
    // Since all coefficients in the polynomials are 1 or 0, we can do the calculation by bit
    // operations. We don't care if cofficients are positive or negative.
    static int calculateBCHCode(int value, int poly);

    // Make bit vector of version information. On success, store the result in "bits" and return true.
    // See 8.10 of JISX0510:2004 (p.45) for details.
    static void makeVersionInfoBits(const Version& version, BitArray& bits);

    // Make bit vector of type information. On success, store the result in "bits" and return true.
    // Encode error correction level and mask pattern. See 8.9 of
    // JISX0510:2004 (p.45) for details.
    static void makeTypeInfoBits(const ErrorCorrectionLevel& ecLevel, int maskPattern, BitArray& bits);

    // Build 2D matrix of QR Code from "dataBits" with "ecLevel", "version" and "getMaskPattern". On
    // success, store the result in "matrix" and return true.
    static void buildMatrix(const BitArray& dataBits,
                            const ErrorCorrectionLevel &ecLevel,
                            Version& version,
                            int maskPattern,
                            ByteMatrix& matrix);
};

}
}

#endif //MATRIXUTIL_H
