// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_GREYSCALE_LUMINANCE_SOURCE
#define ZXING_GREYSCALE_LUMINANCE_SOURCE
/*
 *  GreyscaleLuminanceSource.h
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

#include <zxing/LuminanceSource.h>

namespace zxing {

class GreyscaleLuminanceSource : public LuminanceSource {

private:
  typedef LuminanceSource Super;
  ArrayRef<zxing::byte> greyData_;
  const int dataWidth_;
  const int dataHeight_;
  const int left_;
  const int top_;

public:
  GreyscaleLuminanceSource(ArrayRef<zxing::byte> greyData, int dataWidth, int dataHeight, int left,
                           int top, int width, int height);

  ArrayRef<zxing::byte> getRow(int y, ArrayRef<zxing::byte> row) const;
  ArrayRef<zxing::byte> getMatrix() const;

  bool isRotateSupported() const {
    return true;
  }

  Ref<LuminanceSource> rotateCounterClockwise() const;
};

}

#endif // ZXING_GREYSCALE_LUMINANCE_SOURCE
