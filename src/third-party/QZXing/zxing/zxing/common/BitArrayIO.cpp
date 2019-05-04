// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2010 ZXing authors. All rights reserved.
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

#include <zxing/common/BitArray.h>

using zxing::BitArray;
using std::ostream;

ostream& zxing::operator << (ostream& os, BitArray const& ba) {
  for (int i = 0, size = ba.getSize(); i < size; i++) {
    if ((i & 0x07) == 0) {
      os << ' ';
    }
    os << (ba.get(i) ? 'X' : '.');
  }
  return os;
}
