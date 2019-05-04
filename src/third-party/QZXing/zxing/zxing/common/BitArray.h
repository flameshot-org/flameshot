// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_BIT_ARRAY_H
#define ZXING_BIT_ARRAY_H

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

#include <zxing/ZXing.h>
#include <zxing/common/Counted.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/common/Array.h>
#include <vector>
#include <limits>
#include <iostream>

namespace zxing {

class BitArray : public Counted {
public:
    static const int bitsPerWord = std::numeric_limits<unsigned int>::digits;

private:
    int size;
    ArrayRef<int> bits;
    static const int logBits = ZX_LOG_DIGITS(bitsPerWord);
    static const int bitsMask = (1 << logBits) - 1;

public:
    BitArray();
    BitArray(int size);
    BitArray(std::vector<int> other);
    ~BitArray();
    int getSize() const;
    int getSizeInBytes() const;

    bool get(int i) const {
        return (bits[i / 32] & (1 << (i & 0x1F))) != 0;
    }

    void set(int i) {
        bits[i / 32] |= 1 << (i & 0x1F);
    }

    void flip(int i) {
        bits[i / 32] ^= 1 << (i & 0x1F);
      }

    int getNextSet(int from);
    int getNextUnset(int from);

    void setBulk(int i, int newBits);
    void setRange(int start, int end);
    void clear();
    bool isRange(int start, int end, bool value);
    std::vector<int>& getBitArray();

    void appendBit(bool bit);
    void appendBits(int value, int numBits);
    void appendBitArray(const BitArray& other);
    void ensureCapacity(int size);

    void xor_(const BitArray& other);

    void toBytes(int bitOffset, std::vector<zxing::byte>& array, int offset, int numBytes) const;

    const std::string toString() const;

    static ArrayRef<int> makeArray(int size) {
        return ArrayRef<int>((size + 31) / 32);
      }

    void reverse();

    class Reverse {
    private:
        Ref<BitArray> array;
    public:
        Reverse(Ref<BitArray> array);
        ~Reverse();
    };

private:
    static int makeArraySize(int size);
};

std::ostream& operator << (std::ostream&, BitArray const&);

}

#endif // ZXING_BIT_ARRAY_H
