#include "MaskUtil.h"
#include <zxing/common/IllegalArgumentException.h>
#include <stdlib.h>
#include <algorithm>

namespace zxing {
namespace qrcode {

// Penalty weights from section 6.8.2.1
const int MaskUtil::N1 = 3;
const int MaskUtil::N2 = 3;
const int MaskUtil::N3 = 40;
const int MaskUtil::N4 = 10;

/**
   * Apply mask penalty rule 1 and return the penalty. Find repetitive cells with the same color and
   * give penalty to them. Example: 00000 or 11111.
   */
int MaskUtil::applyMaskPenaltyRule1(const ByteMatrix& matrix)
{
    return applyMaskPenaltyRule1Internal(matrix, true) + applyMaskPenaltyRule1Internal(matrix, false);
}

/**
   * Apply mask penalty rule 2 and return the penalty. Find 2x2 blocks with the same color and give
   * penalty to them. This is actually equivalent to the spec's rule, which is to find MxN blocks and give a
   * penalty proportional to (M-1)x(N-1), because this is the number of 2x2 blocks inside such a block.
   */
int MaskUtil::applyMaskPenaltyRule2(const ByteMatrix& matrix)
{
    int penalty = 0;
    const std::vector<std::vector<zxing::byte> >& array = matrix.getArray();
    size_t width = matrix.getWidth();
    size_t height = matrix.getHeight();
    for (size_t  y = 0; y < height - 1; y++) {
        const std::vector<zxing::byte>& arrayY = array[y];
        for (size_t  x = 0; x < width - 1; x++) {
            int value = arrayY[x];
            if (value == arrayY[x + 1] && value == array[y + 1][x] && value == array[y + 1][x + 1]) {
                penalty++;
            }
        }
    }
    return N2 * penalty;
}

/**
   * Apply mask penalty rule 3 and return the penalty. Find consecutive runs of 1:1:3:1:1:4
   * starting with black, or 4:1:1:3:1:1 starting with white, and give penalty to them.  If we
   * find patterns like 000010111010000, we give penalty once.
   */
int MaskUtil::applyMaskPenaltyRule3(const ByteMatrix& matrix)
{
    int numPenalties = 0;
    const std::vector<std::vector<zxing::byte> >& array = matrix.getArray();
    int width = int(matrix.getWidth());
    int height = int(matrix.getHeight());
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const std::vector<zxing::byte>& arrayY = array[size_t(y)];  // We can at least optimize this access
            if (x + 6 < width &&
                    arrayY[size_t(x)] == 1 &&
                    arrayY[size_t(x + 1)] == 0 &&
                    arrayY[size_t(x + 2)] == 1 &&
                    arrayY[size_t(x + 3)] == 1 &&
                    arrayY[size_t(x + 4)] == 1 &&
                    arrayY[size_t(x + 5)] == 0 &&
                    arrayY[size_t(x + 6)] == 1 &&
                    (isWhiteHorizontal(arrayY, x - 4, x) || isWhiteHorizontal(arrayY, x + 7, x + 11))) {
                numPenalties++;
            }
            if (y + 6 < height &&
                    array[size_t(y)][size_t(x)] == 1  &&
                    array[size_t(y +  1)][size_t(x)] == 0  &&
                    array[size_t(y +  2)][size_t(x)] == 1  &&
                    array[size_t(y +  3)][size_t(x)] == 1  &&
                    array[size_t(y +  4)][size_t(x)] == 1  &&
                    array[size_t(y +  5)][size_t(x)] == 0  &&
                    array[size_t(y +  6)][size_t(x)] == 1 &&
                    (isWhiteVertical(array, x, y - 4, y) || isWhiteVertical(array, x, y + 7, y + 11))) {
                numPenalties++;
            }
        }
    }
    return numPenalties * N3;
}

bool MaskUtil::isWhiteHorizontal(const std::vector<zxing::byte>& rowArray, int from, int to)
{
    from = std::max(from, 0);
    to = std::min(to, int(rowArray.size()));
    for (int i = from; i < to; i++) {
        if (rowArray[size_t(i)] == 1) {
            return false;
        }
    }
    return true;
}

bool MaskUtil::isWhiteVertical(const std::vector<std::vector<zxing::byte> > &array, int col, int from, int to)
{
    from = std::max(from, 0);
    to = std::min(to, int(array.size()));
    for (int i = from; i < to; i++) {
        if (array[size_t(i)][size_t(col)] == 1) {
            return false;
        }
    }
    return true;
}

/**
   * Apply mask penalty rule 4 and return the penalty. Calculate the ratio of dark cells and give
   * penalty if the ratio is far from 50%. It gives 10 penalty for 5% distance.
   */
int MaskUtil::applyMaskPenaltyRule4(const ByteMatrix& matrix)
{
    int numDarkCells = 0;
    const std::vector<std::vector<zxing::byte> >& array = matrix.getArray();
    size_t width = matrix.getWidth();
    size_t height = matrix.getHeight();
    for (size_t y = 0; y < height; y++) {
        const std::vector<zxing::byte>& arrayY = array[y];
        for (size_t x = 0; x < width; x++) {
            if (arrayY[x] == 1) {
                numDarkCells++;
            }
        }
    }
    int numTotalCells = int(matrix.getHeight() * matrix.getWidth());
    int fivePercentVariances = ::abs(numDarkCells * 2 - numTotalCells) * 10 / numTotalCells;
    return fivePercentVariances * N4;
}

/**
   * Return the mask bit for "getMaskPattern" at "x" and "y". See 8.8 of JISX0510:2004 for mask
   * pattern conditions.
   */
bool MaskUtil::getDataMaskBit(int maskPattern, int x, int y)
{
    int intermediate;
    int temp;
    switch (maskPattern) {
    case 0:
        intermediate = (y + x) & 0x1;
        break;
    case 1:
        intermediate = y & 0x1;
        break;
    case 2:
        intermediate = x % 3;
        break;
    case 3:
        intermediate = (y + x) % 3;
        break;
    case 4:
        intermediate = ((y / 2) + (x / 3)) & 0x1;
        break;
    case 5:
        temp = y * x;
        intermediate = (temp & 0x1) + (temp % 3);
        break;
    case 6:
        temp = y * x;
        intermediate = ((temp & 0x1) + (temp % 3)) & 0x1;
        break;
    case 7:
        temp = y * x;
        intermediate = ((temp % 3) + ((y + x) & 0x1)) & 0x1;
        break;
    default:
        throw IllegalArgumentException("Invalid mask pattern");
    }
    return intermediate == 0;
}

/**
   * Helper function for applyMaskPenaltyRule1. We need this for doing this calculation in both
   * vertical and horizontal orders respectively.
   */
int MaskUtil::applyMaskPenaltyRule1Internal(const ByteMatrix& matrix, bool isHorizontal)
{
    int penalty = 0;
    int iLimit = int(isHorizontal ? matrix.getHeight() : matrix.getWidth());
    int jLimit = int(isHorizontal ? matrix.getWidth() : matrix.getHeight());
    const std::vector<std::vector<zxing::byte> >& array = matrix.getArray();
    for (int i = 0; i < iLimit; i++) {
        int numSameBitCells = 0;
        int prevBit = -1;
        for (int j = 0; j < jLimit; j++) {
            int bit = isHorizontal ? array[size_t(i)][size_t(j)] : array[size_t(j)][size_t(i)];
            if (bit == prevBit) {
                numSameBitCells++;
            } else {
                if (numSameBitCells >= 5) {
                    penalty += N1 + (numSameBitCells - 5);
                }
                numSameBitCells = 1;  // Include the cell itself.
                prevBit = bit;
            }
        }
        if (numSameBitCells >= 5) {
            penalty += N1 + (numSameBitCells - 5);
        }
    }
    return penalty;
}

}
}
