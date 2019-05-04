// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_CODE_93_READER_H
#define ZXING_CODE_93_READER_H
/*
 *  Code93Reader.h
 *  ZXing
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

#include <zxing/oned/OneDReader.h>
#include <zxing/common/BitArray.h>
#include <zxing/Result.h>

namespace zxing {
namespace oned {

/**
 * <p>Decodes Code 93 barcodes. This does not support "Full ASCII Code 93" yet.</p>
 * Ported form Java (author Sean Owen)
 * @author Lukasz Warchol
 */
class Code93Reader : public OneDReader {
public:
  Code93Reader();
  Ref<Result> decodeRow(int rowNumber, Ref<BitArray> row, DecodeHints hints);

private:
  std::string decodeRowResult;
  std::vector<int> counters;

  Range findAsteriskPattern(Ref<BitArray> row);

  static int toPattern(std::vector<int>& counters);
  static char patternToChar(int pattern);
  static Ref<String> decodeExtended(std::string const& encoded);
  static void checkChecksums(std::string const& result);
  static void checkOneChecksum(std::string const& result,
                               int checkPosition,
                               int weightMax);
};

}
}

#endif // ZXING_CODE_93_READER_H

