#ifndef MASKUTIL_H
#define MASKUTIL_H

#include "ByteMatrix.h"
#include <vector>

namespace zxing {
namespace qrcode {

class MaskUtil {

private:
  static const int N1;
  static const int N2;
  static const int N3;
  static const int N4;

  static bool isWhiteHorizontal(const std::vector<zxing::byte>& rowArray, int from, int to);

  static bool isWhiteVertical(const std::vector<std::vector<zxing::byte> >& array, int col, int from, int to);

  /**
   * Helper function for applyMaskPenaltyRule1. We need this for doing this calculation in both
   * vertical and horizontal orders respectively.
   */
  static int applyMaskPenaltyRule1Internal(const ByteMatrix& matrix, bool isHorizontal);

public:
  /**
   * Apply mask penalty rule 1 and return the penalty. Find repetitive cells with the same color and
   * give penalty to them. Example: 00000 or 11111.
   */
  static int applyMaskPenaltyRule1(const ByteMatrix& matrix);

  /**
   * Apply mask penalty rule 2 and return the penalty. Find 2x2 blocks with the same color and give
   * penalty to them. This is actually equivalent to the spec's rule, which is to find MxN blocks and give a
   * penalty proportional to (M-1)x(N-1), because this is the number of 2x2 blocks inside such a block.
   */
  static int applyMaskPenaltyRule2(const ByteMatrix& matrix);

  /**
   * Apply mask penalty rule 3 and return the penalty. Find consecutive runs of 1:1:3:1:1:4
   * starting with black, or 4:1:1:3:1:1 starting with white, and give penalty to them.  If we
   * find patterns like 000010111010000, we give penalty once.
   */
  static int applyMaskPenaltyRule3(const ByteMatrix& matrix);

  /**
   * Apply mask penalty rule 4 and return the penalty. Calculate the ratio of dark cells and give
   * penalty if the ratio is far from 50%. It gives 10 penalty for 5% distance.
   */
  static int applyMaskPenaltyRule4(const ByteMatrix& matrix);

  /**
   * Return the mask bit for "getMaskPattern" at "x" and "y". See 8.8 of JISX0510:2004 for mask
   * pattern conditions.
   */
  static bool getDataMaskBit(int maskPattern, int x, int y);
};

}
}

#endif // MASKUTIL_H
