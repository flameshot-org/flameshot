// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_GENERIC_MULTIPLE_BARCODE_READER_H
#define ZXING_GENERIC_MULTIPLE_BARCODE_READER_H

/*
 *  Copyright 2011 ZXing authors All rights reserved.
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

#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/Reader.h>

namespace zxing {
namespace multi {

class GenericMultipleBarcodeReader : public MultipleBarcodeReader {
 private:
  static Ref<Result> translateResultPoints(Ref<Result> result, 
                                           int xOffset, 
                                           int yOffset);
  void doDecodeMultiple(Ref<BinaryBitmap> image, 
                        DecodeHints hints, 
                        std::vector<Ref<Result> >& results, 
                        int xOffset, 
                        int yOffset,
                        int currentDepth);
  Reader& delegate_;
  static const int MIN_DIMENSION_TO_RECUR = 100;
  static const int MAX_DEPTH = 4;

 public:
  GenericMultipleBarcodeReader(Reader& delegate);
  virtual ~GenericMultipleBarcodeReader();
  virtual std::vector<Ref<Result> > decodeMultiple(Ref<BinaryBitmap> image, DecodeHints hints);
};

}
}

#endif // ZXING_GENERIC_MULTIPLE_BARCODE_READER_H
