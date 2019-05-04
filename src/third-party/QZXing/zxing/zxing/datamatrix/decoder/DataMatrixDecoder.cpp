/*
 *  Decoder.cpp
 *  zxing
 *
 *  Created by Luiz Silva on 09/02/2010.
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

#include <zxing/datamatrix/decoder/Decoder.h>
#include <zxing/datamatrix/decoder/BitMatrixParser.h>
#include <zxing/datamatrix/decoder/DataBlock.h>
#include <zxing/datamatrix/decoder/DecodedBitStreamParser.h>
#include <zxing/datamatrix/Version.h>
#include <zxing/ReaderException.h>
#include <zxing/ChecksumException.h>
#include <zxing/common/reedsolomon/ReedSolomonException.h>

using zxing::Ref;
using zxing::DecoderResult;
//using zxing::datamatrix::Decoder;
//using zxing::datamatrix::DataBlock;
//using zxing::datamatrix::DecodedBitStreamParser;

// VC++
using zxing::ArrayRef;
using zxing::BitMatrix;

namespace zxing {
namespace datamatrix {

Decoder::Decoder() : rsDecoder_(GenericGF::DATA_MATRIX_FIELD_256) {}

void Decoder::correctErrors(ArrayRef<zxing::byte> codewordBytes, int numDataCodewords) {
  int numCodewords = codewordBytes->size();
  ArrayRef<int> codewordInts(numCodewords);
  for (int i = 0; i < numCodewords; i++) {
    codewordInts[i] = codewordBytes[i] & 0xff;
  }
  int numECCodewords = numCodewords - numDataCodewords;
  try {
    rsDecoder_.decode(codewordInts, numECCodewords);
  } catch (ReedSolomonException const& ignored) {
    (void)ignored;
    throw ChecksumException();
  }
  // Copy back into array of bytes -- only need to worry about the bytes that were data
  // We don't care about errors in the error-correction codewords
  for (int i = 0; i < numDataCodewords; i++) {
    codewordBytes[i] = (zxing::byte)codewordInts[i];
  }
}

Ref<DecoderResult> Decoder::decode(Ref<BitMatrix> bits) {
  // Construct a parser and read version, error-correction level
  BitMatrixParser parser(bits);
  Version *version = parser.readVersion(bits);

  // Read codewords
  ArrayRef<zxing::byte> codewords(parser.readCodewords());
  // Separate into data blocks
  std::vector<Ref<DataBlock> > dataBlocks = DataBlock::getDataBlocks(codewords, version);

  int dataBlocksCount = int(dataBlocks.size());

  // Count total number of data bytes
  int totalBytes = 0;
  for (int i = 0; i < dataBlocksCount; i++) {
    totalBytes += dataBlocks[i]->getNumDataCodewords();
  }
  ArrayRef<zxing::byte> resultBytes(totalBytes);

  // Error-correct and copy data blocks together into a stream of bytes
  for (int j = 0; j < dataBlocksCount; j++) {
    Ref<DataBlock> dataBlock(dataBlocks[j]);
    ArrayRef<zxing::byte> codewordBytes = dataBlock->getCodewords();
    int numDataCodewords = dataBlock->getNumDataCodewords();
    correctErrors(codewordBytes, numDataCodewords);
    for (int i = 0; i < numDataCodewords; i++) {
      // De-interlace data blocks.
      resultBytes[i * dataBlocksCount + j] = codewordBytes[i];
    }
  }
  // Decode the contents of that stream of bytes
  DecodedBitStreamParser decodedBSParser;
  return Ref<DecoderResult> (decodedBSParser.decode(resultBytes));
}

}
}
