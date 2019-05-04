// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_HYBRIDBINARIZER_H
#define ZXING_HYBRIDBINARIZER_H
/*
 *  HybridBinarizer.h
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

#include <vector>
#include <zxing/Binarizer.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/BitArray.h>
#include <zxing/common/BitMatrix.h>

namespace zxing {
	
	class HybridBinarizer : public GlobalHistogramBinarizer {
	 private:
    Ref<BitMatrix> matrix_;
	  Ref<BitArray> cached_row_;

	public:
		HybridBinarizer(Ref<LuminanceSource> source);
		virtual ~HybridBinarizer();
		
		virtual Ref<BitMatrix> getBlackMatrix();
		Ref<Binarizer> createBinarizer(Ref<LuminanceSource> source);
  private:
    // We'll be using one-D arrays because C++ can't dynamically allocate 2D
    // arrays
    ArrayRef<int> calculateBlackPoints(ArrayRef<zxing::byte> luminances,
                                       int subWidth,
                                       int subHeight,
                                       int width,
                                       int height);
    void calculateThresholdForBlock(ArrayRef<zxing::byte> luminances,
                                    int subWidth,
                                    int subHeight,
                                    int width,
                                    int height,
                                    ArrayRef<int> blackPoints,
                                    Ref<BitMatrix> const& matrix);
    void thresholdBlock(ArrayRef<zxing::byte>luminances,
                        int xoffset,
                        int yoffset,
                        int threshold,
                        int stride,
                        Ref<BitMatrix> const& matrix);
	};

}

#endif // ZXING_HYBRIDBINARIZER_H
