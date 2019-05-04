#ifndef ZXING_DECOCER_PDF_H
#define ZXING_DECOCER_PDF_H

/*
 *  Decoder.h
 *  zxing
 *
 *  Created by Hartmut Neubauer, 2012-05-25
 *  Copyright 2010,2012 ZXing authors All rights reserved.
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

#include <zxing/pdf417/decoder/ec/ErrorCorrection.h>
#include <zxing/pdf417/decoder/ec/ModulusGF.h>
#include <zxing/common/Counted.h>
#include <zxing/common/Array.h>
#include <zxing/common/DecoderResult.h>
#include <zxing/common/BitMatrix.h>


namespace zxing {
namespace pdf417 {
namespace decoder {

/**
 * <p>The main class which implements PDF417 Code decoding -- as
 * opposed to locating and extracting the PDF417 Code from an image.</p>
 *
 * <p> 2012-06-27 HFN Reed-Solomon error correction activated, see class PDF417RSDecoder. </p>
 * <p> 2012-09-19 HFN Reed-Solomon error correction via ErrorCorrection/ModulusGF/ModulusPoly. </p>
 */

class Decoder {
private:
  static const int MAX_ERRORS;
  static const int MAX_EC_CODEWORDS;

  void correctErrors(ArrayRef<int> codewords,
		ArrayRef<int> erasures, int numECCodewords);
  static void verifyCodewordCount(ArrayRef<int> codewords, int numECCodewords);

public:

  Ref<DecoderResult> decode(Ref<BitMatrix> bits, DecodeHints const &hints);
};

}
}
}

#endif // ZXING_DECOCER_PDF_H
