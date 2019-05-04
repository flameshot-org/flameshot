// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_UPC_E_READER_H
#define ZXING_UPC_E_READER_H

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

#include <zxing/oned/UPCEANReader.h>
#include <zxing/Result.h>

namespace zxing {
namespace oned {

class UPCEReader : public UPCEANReader {
private:
  std::vector<int> decodeMiddleCounters;
  static bool determineNumSysAndCheckDigit(std::string& resultString, int lgPatternFound);

protected:
  Range decodeEnd(Ref<BitArray> row, int endStart);
  bool checkChecksum(Ref<String> const& s);
public:
  UPCEReader();

  int decodeMiddle(Ref<BitArray> row, Range const& startRange, std::string& resultString);
  static Ref<String> convertUPCEtoUPCA(Ref<String> const& upce);

  BarcodeFormat getBarcodeFormat();
};

}
}

#endif // ZXING_UPC_E_READER_H

