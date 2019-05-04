// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  LuminanceSource.cpp
 *  zxing
 *
 *  Copyright 2008 ZXing authors All rights reserved.
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

#include <sstream>
#include <zxing/LuminanceSource.h>
#include <zxing/InvertedLuminanceSource.h>
#include <zxing/common/IllegalArgumentException.h>

using zxing::Ref;
using zxing::LuminanceSource;

LuminanceSource::LuminanceSource(int width_, int height_) :width(width_), height(height_) {}

LuminanceSource::~LuminanceSource() {}

bool LuminanceSource::isCropSupported() const {
  return false;
}

Ref<LuminanceSource> LuminanceSource::crop(int, int, int, int) const {
  throw IllegalArgumentException("This luminance source does not support cropping.");
}

bool LuminanceSource::isRotateSupported() const {
  return false;
}

Ref<LuminanceSource> LuminanceSource::rotateCounterClockwise() const {
    throw IllegalArgumentException("This luminance source does not support rotation.");
}

Ref<zxing::LuminanceSource> LuminanceSource::rotateCounterClockwise45() const
{
    throw IllegalArgumentException("This luminance source does not support rotation 45.");
}

LuminanceSource::operator std::string() const {
  ArrayRef<zxing::byte> row;
  std::ostringstream oss;
  for (int y = 0; y < getHeight(); y++) {
    row = getRow(y, row);
    for (int x = 0; x < getWidth(); x++) {
      int luminance = row[x] & 0xFF;
      char c;
      if (luminance < 0x40) {
        c = '#';
      } else if (luminance < 0x80) {
        c = '+';
      } else if (luminance < 0xC0) {
        c = '.';
      } else {
        c = ' ';
      }
      oss << c;
    }
    oss << '\n';
  }
  return oss.str();
}

Ref<LuminanceSource> LuminanceSource::invert() const {

  // N.B.: this only works because we use counted objects with the
  // count in the object. This is _not_ how things like shared_ptr
  // work. They do not allow you to make a smart pointer from a native
  // pointer more than once. If we ever switch to (something like)
  // shared_ptr's, the luminace source is going to have keep a weak
  // pointer to itself from which it can create a strong pointer as
  // needed. And, FWIW, that has nasty semantics in the face of
  // exceptions during construction.

  return Ref<LuminanceSource>
      (new InvertedLuminanceSource(Ref<LuminanceSource>(const_cast<LuminanceSource*>(this))));
}
