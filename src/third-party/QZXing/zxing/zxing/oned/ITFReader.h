// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_ITF_READER_H
#define ZXING_ITF_READER_H

/*
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

class ITFReader : public OneDReader {
private:
  enum {MAX_AVG_VARIANCE = (unsigned int) (PATTERN_MATCH_RESULT_SCALE_FACTOR * 420/1000)};
  enum {MAX_INDIVIDUAL_VARIANCE = (int) (PATTERN_MATCH_RESULT_SCALE_FACTOR * 780/1000)};
  // Stores the actual narrow line width of the image being decoded.
  int narrowLineWidth;
			
  Range decodeStart(Ref<BitArray> row);
  Range decodeEnd(Ref<BitArray> row);
  static void decodeMiddle(Ref<BitArray> row, int payloadStart, int payloadEnd, std::string& resultString);
  void validateQuietZone(Ref<BitArray> row, int startPattern);
  static int skipWhiteSpace(Ref<BitArray> row);
			
  static Range findGuardPattern(Ref<BitArray> row, int rowOffset, std::vector<int> const& pattern);
  static int decodeDigit(std::vector<int>& counters);
			
  void append(char* s, char c);
public:
  Ref<Result> decodeRow(int rowNumber, Ref<BitArray> row, DecodeHints hints);
  ITFReader();
  ~ITFReader();
};

}
}

#endif // ZXING_ITF_READER_H

