#ifndef ZXING_BIT_SOURCE_H
#define ZXING_BIT_SOURCE_H

/*
 *  BitSource.h
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

#include <zxing/common/Array.h>
#include <zxing/common/Types.h>

namespace zxing {
/**
 * <p>This provides an easy abstraction to read bits at a time from a sequence of bytes, where the
 * number of bits read is not often a multiple of 8.</p>
 *
 * <p>This class is not thread-safe.</p>
 *
 * @author srowen@google.com (Sean Owen)
 * @author christian.brunschen@gmail.com (Christian Brunschen)
 */
class BitSource : public Counted {
private:
  ArrayRef<zxing::byte> bytes_;
  int byteOffset_;
  int bitOffset_;
public:
  /**
   * @param bytes bytes from which this will read bits. Bits will be read from the first byte first.
   * Bits are read within a byte from most-significant to least-significant bit.
   */
  BitSource(ArrayRef<zxing::byte> &bytes) :
      bytes_(bytes), byteOffset_(0), bitOffset_(0) {
  }

  int getBitOffset() {
    return bitOffset_;
  }

  int getByteOffset() {
    return byteOffset_;
  }

  /**
   * @param numBits number of bits to read
   * @return int representing the bits read. The bits will appear as the least-significant
   *         bits of the int
   * @throws IllegalArgumentException if numBits isn't in [1,32]
   */
  int readBits(int numBits);

  /**
   * @return number of bits that can be read successfully
   */
  int available();
};

}

#endif // ZXING_BIT_SOURCE_H
