// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Detector.h
 *  zxing
 *
 *  Created by Lukas Stabe on 08/02/2012.
 *  Copyright 2012 ZXing authors All rights reserved.
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

#ifndef ZXING_AZTEC_DETECTOR_DETECTOR_H
#define ZXING_AZTEC_DETECTOR_DETECTOR_H

#include <vector>

#include <zxing/common/BitArray.h>
#include <zxing/ResultPoint.h>
#include <zxing/common/BitMatrix.h>
#include <zxing/DecodeHints.h>
#include <zxing/aztec/AztecDetectorResult.h>

namespace zxing {
namespace aztec {

class Point : public Counted {
 private:
  const int x;
  const int y;
            
 public:
  Ref<ResultPoint> toResultPoint() { 
    return Ref<ResultPoint>(new ResultPoint(float(x), float(y)));
  }
            
  Point(int ax, int ay) : x(ax), y(ay) {}

  int getX() const { return x; }
  int getY() const { return y; }
};
        
class Detector : public Counted {
            
 private:
  Ref<BitMatrix> image_;
            
  bool compact_;
  int nbLayers_;
  int nbDataBlocks_;
  int nbCenterLayers_;
  int shift_;
            
  void extractParameters(std::vector<Ref<Point> > bullEyeCornerPoints);
  ArrayRef< Ref<ResultPoint> > getMatrixCornerPoints(std::vector<Ref<Point> > bullEyeCornerPoints);
  static void correctParameterData(Ref<BitArray> parameterData, bool compact);
  std::vector<Ref<Point> > getBullEyeCornerPoints(Ref<Point> pCenter);
  Ref<Point> getMatrixCenter();
  Ref<BitMatrix> sampleGrid(Ref<BitMatrix> image,
                            Ref<ResultPoint> topLeft,
                            Ref<ResultPoint> bottomLeft,
                            Ref<ResultPoint> bottomRight,
                            Ref<ResultPoint> topRight);
  void getParameters(Ref<BitArray> parameterData);
  Ref<BitArray> sampleLine(Ref<Point> p1, Ref<Point> p2, int size);
  bool isWhiteOrBlackRectangle(Ref<Point> p1,
                               Ref<Point> p2,
                               Ref<Point> p3,
                               Ref<Point> p4);
  int getColor(Ref<Point> p1, Ref<Point> p2);
  Ref<Point> getFirstDifferent(Ref<Point> init, bool color, int dx, int dy);
  bool isValid(int x, int y);
  static float distance(Ref<Point> a, Ref<Point> b);
            
 public:
  Detector(Ref<BitMatrix> image);
  Ref<AztecDetectorResult> detect();
};

}
}

#endif // ZXING_AZTEC_DETECTOR_DETECTOR_H
