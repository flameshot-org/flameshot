/*
 *  PerspectiveTransform.cpp
 *  zxing
 *
 *  Created by Christian Brunschen on 12/05/2008.
 *  Copyright 2008 Google UK. All rights reserved.
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

#include <zxing/common/PerspectiveTransform.h>

namespace zxing {
using namespace std;

PerspectiveTransform::PerspectiveTransform(float inA11, float inA21, 
                                           float inA31, float inA12, 
                                           float inA22, float inA32, 
                                           float inA13, float inA23, 
                                           float inA33) : 
  a11(inA11), a12(inA12), a13(inA13), a21(inA21), a22(inA22), a23(inA23),
  a31(inA31), a32(inA32), a33(inA33) {}

Ref<PerspectiveTransform> PerspectiveTransform::quadrilateralToQuadrilateral(float x0, float y0, float x1, float y1,
    float x2, float y2, float x3, float y3, float x0p, float y0p, float x1p, float y1p, float x2p, float y2p,
    float x3p, float y3p) {
  Ref<PerspectiveTransform> qToS = PerspectiveTransform::quadrilateralToSquare(x0, y0, x1, y1, x2, y2, x3, y3);
  Ref<PerspectiveTransform> sToQ =
    PerspectiveTransform::squareToQuadrilateral(x0p, y0p, x1p, y1p, x2p, y2p, x3p, y3p);
  return sToQ->times(qToS);
}

Ref<PerspectiveTransform> PerspectiveTransform::squareToQuadrilateral(float x0, float y0, float x1, float y1, float x2,
    float y2, float x3, float y3) {
  float dx3 = x0 - x1 + x2 - x3;
  float dy3 = y0 - y1 + y2 - y3;
  if (dx3 == 0.0f && dy3 == 0.0f) {
    Ref<PerspectiveTransform> result(new PerspectiveTransform(x1 - x0, x2 - x1, x0, y1 - y0, y2 - y1, y0, 0.0f,
                                     0.0f, 1.0f));
    return result;
  } else {
    float dx1 = x1 - x2;
    float dx2 = x3 - x2;
    float dy1 = y1 - y2;
    float dy2 = y3 - y2;
    float denominator = dx1 * dy2 - dx2 * dy1;
    float a13 = (dx3 * dy2 - dx2 * dy3) / denominator;
    float a23 = (dx1 * dy3 - dx3 * dy1) / denominator;
    Ref<PerspectiveTransform> result(new PerspectiveTransform(x1 - x0 + a13 * x1, x3 - x0 + a23 * x3, x0, y1 - y0
                                     + a13 * y1, y3 - y0 + a23 * y3, y0, a13, a23, 1.0f));
    return result;
  }
}

Ref<PerspectiveTransform> PerspectiveTransform::quadrilateralToSquare(float x0, float y0, float x1, float y1, float x2,
    float y2, float x3, float y3) {
  // Here, the adjoint serves as the inverse:
  return squareToQuadrilateral(x0, y0, x1, y1, x2, y2, x3, y3)->buildAdjoint();
}

Ref<PerspectiveTransform> PerspectiveTransform::buildAdjoint() {
  // Adjoint is the transpose of the cofactor matrix:
  Ref<PerspectiveTransform> result(new PerspectiveTransform(a22 * a33 - a23 * a32, a23 * a31 - a21 * a33, a21 * a32
                                   - a22 * a31, a13 * a32 - a12 * a33, a11 * a33 - a13 * a31, a12 * a31 - a11 * a32, a12 * a23 - a13 * a22,
                                   a13 * a21 - a11 * a23, a11 * a22 - a12 * a21));
  return result;
}

Ref<PerspectiveTransform> PerspectiveTransform::times(Ref<PerspectiveTransform> other) {
  Ref<PerspectiveTransform> result(new PerspectiveTransform(a11 * other->a11 + a21 * other->a12 + a31 * other->a13,
                                   a11 * other->a21 + a21 * other->a22 + a31 * other->a23, a11 * other->a31 + a21 * other->a32 + a31
                                   * other->a33, a12 * other->a11 + a22 * other->a12 + a32 * other->a13, a12 * other->a21 + a22
                                   * other->a22 + a32 * other->a23, a12 * other->a31 + a22 * other->a32 + a32 * other->a33, a13
                                   * other->a11 + a23 * other->a12 + a33 * other->a13, a13 * other->a21 + a23 * other->a22 + a33
                                   * other->a23, a13 * other->a31 + a23 * other->a32 + a33 * other->a33));
  return result;
}

void PerspectiveTransform::transformPoints(vector<float> &points) {
  int max = int(points.size());
  for (int i = 0; i < max; i += 2) {
    float x = points[i];
    float y = points[i + 1];
    float denominator = a13 * x + a23 * y + a33;
    points[i] = (a11 * x + a21 * y + a31) / denominator;
    points[i + 1] = (a12 * x + a22 * y + a32) / denominator;
  }
}

ostream& operator<<(ostream& out, const PerspectiveTransform &pt) {
  out << pt.a11 << ", " << pt.a12 << ", " << pt.a13 << ", \n";
  out << pt.a21 << ", " << pt.a22 << ", " << pt.a23 << ", \n";
  out << pt.a31 << ", " << pt.a32 << ", " << pt.a33 << "\n";
  return out;
}

}
