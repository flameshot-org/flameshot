// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_BIT_MATRIX_H
#define ZXING_BIT_MATRIX_H

/*
 *  BitMatrix.h
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
#include <zxing/common/BitArray.h>
#include <zxing/common/Array.h>
#include <limits>

namespace zxing {

class BitMatrix : public Counted {
public:
  static const int bitsPerWord = std::numeric_limits<unsigned int>::digits;

private:
  int width;
  int height;
  int rowSize;
  ArrayRef<int> bits;

public:
  BitMatrix(int dimension);
  BitMatrix(int width, int height);

  ~BitMatrix();

  bool get(int x, int y) const {
    int offset = y * rowSize + (x >> 5);
    return ((((unsigned)bits[offset]) >> (x & 0x1f)) & 1) != 0;
  }

  void set(int x, int y) {
    int offset = y * rowSize + (x >> 5);
    bits[offset] |= 1 << (x & 0x1f);
  }

  void flip(int x, int y);
  void rotate180();

  void clear();
  void setRegion(int left, int top, int width, int height);
  Ref<BitArray> getRow(int y, Ref<BitArray> row);
  void setRow(int y, Ref<BitArray> row);

  int getWidth() const;
  int getHeight() const;

  ArrayRef<int> getTopLeftOnBit() const;
  ArrayRef<int> getBottomRightOnBit() const;
  ArrayRef<int> getEnclosingRectangle() const;

  friend std::ostream& operator<<(std::ostream &out, const BitMatrix &bm);
  const char *description();

private:
  inline void init(int, int);

  BitMatrix(const BitMatrix&);
  BitMatrix& operator =(const BitMatrix&);
};

}

#endif // ZXING_BIT_MATRIX_H
