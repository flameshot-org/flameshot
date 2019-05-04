#ifndef ZXING_PERSPECTIVE_TANSFORM_H
#define ZXING_PERSPECTIVE_TANSFORM_H

/*
 *  PerspectiveTransform.h
 *  zxing
 *
 *  Copyright 2010 ZXing authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zxing/common/Counted.h>
#include <vector>

namespace zxing {
class PerspectiveTransform : public Counted {
private:
  float a11, a12, a13, a21, a22, a23, a31, a32, a33;
  PerspectiveTransform(float a11, float a21, float a31, float a12, float a22, float a32, float a13, float a23,
                       float a33);

public:
  static Ref<PerspectiveTransform>
  quadrilateralToQuadrilateral(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
                               float x0p, float y0p, float x1p, float y1p, float x2p, float y2p, float x3p, float y3p);
  static Ref<PerspectiveTransform> squareToQuadrilateral(float x0, float y0, float x1, float y1, float x2, float y2,
      float x3, float y3);
  static Ref<PerspectiveTransform> quadrilateralToSquare(float x0, float y0, float x1, float y1, float x2, float y2,
      float x3, float y3);
  Ref<PerspectiveTransform> buildAdjoint();
  Ref<PerspectiveTransform> times(Ref<PerspectiveTransform> other);
  void transformPoints(std::vector<float> &points);

  friend std::ostream& operator<<(std::ostream& out, const PerspectiveTransform &pt);
};
}

#endif // ZXING_PERSPECTIVE_TANSFORM_H
