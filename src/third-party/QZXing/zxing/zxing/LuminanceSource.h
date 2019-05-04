// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_LUMINANCESOURCE_H
#define ZXING_LUMINANCESOURCE_H
/*
 *  LuminanceSource.h
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
#include <zxing/common/Array.h>
#include <string.h>
#include "common/Types.h"

namespace zxing {

class LuminanceSource : public Counted {
 private:
  const int width;
  const int height;

 public:
  LuminanceSource(int width, int height);
  virtual ~LuminanceSource();

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  // Callers take ownership of the returned memory and must call delete [] on it themselves.
  virtual ArrayRef<zxing::byte> getRow(int y, ArrayRef<zxing::byte> row) const = 0;
  virtual ArrayRef<zxing::byte> getMatrix() const = 0;

  virtual bool isCropSupported() const;
  virtual Ref<LuminanceSource> crop(int left, int top, int width, int height) const;

  virtual bool isRotateSupported() const;

  virtual Ref<LuminanceSource> invert() const;
  
  virtual Ref<LuminanceSource> rotateCounterClockwise() const;

  virtual Ref<LuminanceSource> rotateCounterClockwise45() const;

  operator std::string () const;
};

}

#endif // ZXING_LUMINANCESOURCE_H
