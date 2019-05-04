// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_INVERTEDLUMINANCESOURCE_H
#define ZXING_INVERTEDLUMINANCESOURCE_H
/*
 *  Copyright 2013 ZXing authors All rights reserved.
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

#include <zxing/ZXing.h>
#include <zxing/LuminanceSource.h>

namespace zxing {

class InvertedLuminanceSource : public LuminanceSource {
private:
  typedef LuminanceSource Super;
  const Ref<LuminanceSource> delegate;

public:
  InvertedLuminanceSource(Ref<LuminanceSource> const&);

  ArrayRef<zxing::byte> getRow(int y, ArrayRef<zxing::byte> row) const;
  ArrayRef<zxing::byte> getMatrix() const;

  boolean isCropSupported() const;
  Ref<LuminanceSource> crop(int left, int top, int width, int height) const;

  boolean isRotateSupported() const;

  virtual Ref<LuminanceSource> invert() const;

  Ref<LuminanceSource> rotateCounterClockwise() const;
};

}

#endif // ZXING_INVERTEDLUMINANCESOURCE_H
