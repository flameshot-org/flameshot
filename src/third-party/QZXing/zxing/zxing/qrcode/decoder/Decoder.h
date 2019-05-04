#ifndef ZXING_DECODER_H
#define ZXING_DECODER_H

/*
 *  Decoder.h
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

#include <zxing/common/reedsolomon/ReedSolomonDecoder.h>
#include <zxing/common/Counted.h>
#include <zxing/common/Array.h>
#include <zxing/common/DecoderResult.h>
#include <zxing/common/BitMatrix.h>

namespace zxing {
namespace qrcode {

class Decoder {
private:
  ReedSolomonDecoder rsDecoder_;

  void correctErrors(ArrayRef<zxing::byte> bytes, int numDataCodewords);

public:
  Decoder();
  Ref<DecoderResult> decode(Ref<BitMatrix> bits);
};

}
}

#endif // ZXING_DECODER_H
