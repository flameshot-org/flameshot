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

#include <zxing/common/BitMatrix.h>
#include <zxing/common/IllegalArgumentException.h>

#include <iostream>
#include <sstream>
#include <string>

using std::ostream;
using std::ostringstream;

using zxing::BitMatrix;
using zxing::BitArray;
using zxing::ArrayRef;
using zxing::Ref;

void BitMatrix::init(int width, int height) {
    if (width < 1 || height < 1) {
        throw IllegalArgumentException("Both dimensions must be greater than 0");
    }
    this->width = width;
    this->height = height;
    this->rowSize = (width + 31) >> 5;
    bits = ArrayRef<int>(rowSize * height);
}

BitMatrix::BitMatrix(int dimension) {
    init(dimension, dimension);
}

BitMatrix::BitMatrix(int width, int height) {
    init(width, height);
}

BitMatrix::~BitMatrix() {}

void BitMatrix::flip(int x, int y) {
    int offset = y * rowSize + (x >> 5);
    bits[offset] ^= 1 << (x & 0x1f);
}

void BitMatrix::rotate180()
{
    int width = getWidth();
    int height = getHeight();
    Ref<BitArray> topRow( new BitArray(width) );
    Ref<BitArray> bottomRow( new BitArray(width) );
    for (int i = 0; i < (height+1) / 2; i++) {
        getRow(i, topRow);
        bottomRow = getRow(height - 1 - i, bottomRow);
        topRow->reverse();
        bottomRow->reverse();
        setRow(i, bottomRow);
        setRow(height - 1 - i, topRow);
    }
}

void BitMatrix::setRegion(int left, int top, int width, int height) {
    if (top < 0 || left < 0) {
        throw IllegalArgumentException("Left and top must be nonnegative");
    }
    if (height < 1 || width < 1) {
        throw IllegalArgumentException("Height and width must be at least 1");
    }
    int right = left + width;
    int bottom = top + height;
    if (bottom > this->height || right > this->width) {
        throw IllegalArgumentException("The region must fit inside the matrix");
    }
    for (int y = top; y < bottom; y++) {
        int offset = y * rowSize;
        for (int x = left; x < right; x++) {
            bits[offset + (x >> 5)] |= 1 << (x & 0x1f);
        }
    }
}

Ref<BitArray> BitMatrix::getRow(int y, Ref<BitArray> row) {
    if (row.empty() || row->getSize() < width) {
        row = new BitArray(width);
    }
    int offset = y * rowSize;
    for (int x = 0; x < rowSize; x++) {
        row->setBulk(x << 5, bits[offset + x]);
    }
    return row;
}

void BitMatrix::setRow(int y, Ref<zxing::BitArray> row)
{
    if (y < 0 || y >= bits->size() ||
            row->getSize() != width)
    {
        throw IllegalArgumentException("setRow arguments invalid");
    }

    //change with memcopy
    for(int i=0; i<width; i++)
        bits[y * rowSize + i] = row->get(i);
}

int BitMatrix::getWidth() const {
    return width;
}

int BitMatrix::getHeight() const {
    return height;
}

ArrayRef<int> BitMatrix::getTopLeftOnBit() const {
    int bitsOffset = 0;
    while (bitsOffset < bits->size() && bits[bitsOffset] == 0) {
        bitsOffset++;
    }
    if (bitsOffset == bits->size()) {
        return ArrayRef<int>();
    }
    int y = bitsOffset / rowSize;
    int x = (bitsOffset % rowSize) << 5;

    int theBits = bits[bitsOffset];
    int bit = 0;
    while ((theBits << (31-bit)) == 0) {
        bit++;
    }
    x += bit;
    ArrayRef<int> res (2);
    res[0]=x;
    res[1]=y;
    return res;
}

ArrayRef<int> BitMatrix::getBottomRightOnBit() const {
    int bitsOffset = bits->size() - 1;
    while (bitsOffset >= 0 && bits[bitsOffset] == 0) {
        bitsOffset--;
    }
    if (bitsOffset < 0) {
        return ArrayRef<int>();
    }

    int y = bitsOffset / rowSize;
    int x = (bitsOffset % rowSize) << 5;

    int theBits = bits[bitsOffset];
    int bit = 31;
    while ((theBits >> bit) == 0) {
        bit--;
    }
    x += bit;

    ArrayRef<int> res (2);
    res[0]=x;
    res[1]=y;
    return res;
}

ArrayRef<int> BitMatrix::getEnclosingRectangle() const
{
    int left = width;
    int top = height;
    int right = -1;
    int bottom = -1;

    for (int y = 0; y < height; y++) {
        for (int x32 = 0; x32 < rowSize; x32++) {
            int theBits = bits[y * rowSize + x32];
            if (theBits != 0) {
                if (y < top) {
                    top = y;
                }
                if (y > bottom) {
                    bottom = y;
                }
                if (x32 * 32 < left) {
                    int bit = 0;
                    while ((theBits << (31 - bit)) == 0) {
                        bit++;
                    }
                    if ((x32 * 32 + bit) < left) {
                        left = x32 * 32 + bit;
                    }
                }
                if (x32 * 32 + 31 > right) {
                    int bit = 31;
                    while ((unsigned(theBits) >> unsigned(bit)) == 0) {
                        bit--;
                    }
                    if ((x32 * 32 + bit) > right) {
                        right = x32 * 32 + bit;
                    }
                }
            }
        }
    }

    int width = right - left;
    int height = bottom - top;

    if (width < 0 || height < 0) {
        return ArrayRef<int>();
    }

    ArrayRef<int> res(4);
    res[0] = left;
    res[1] = top;
    res[2] = width;
    res[3] = height;

    return res;
}
