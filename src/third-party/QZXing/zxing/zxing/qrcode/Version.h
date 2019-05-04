#ifndef ZXING_VERSION_H
#define ZXING_VERSION_H

/*
 *  Version.h
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

#include <zxing/common/Counted.h>
#include <zxing/qrcode/ErrorCorrectionLevel.h>
#include <zxing/ReaderException.h>
#include <zxing/common/BitMatrix.h>
#include <zxing/common/Counted.h>
#include <vector>

namespace zxing {
namespace qrcode {

class ECB {
private:
  int count_;
  int dataCodewords_;
public:
  ECB(int count, int dataCodewords);
  int getCount();
  int getDataCodewords();
};

class ECBlocks {
private:
  int ecCodewordsPerBloc_;
  std::vector<ECB*> ecBlocks_;
public:
  ECBlocks(int ecCodewordsPerBloc, ECB *ecBlocks);
  ECBlocks(int ecCodewordsPerBloc, ECB *ecBlocks1, ECB *ecBlocks2);
  int getECCodewordsPerBloc();
  int getTotalECCodewords();
  std::vector<ECB*>& getECBlocks();
  ~ECBlocks();
};

class Version : public Counted {

private:
  int versionNumber_;
  std::vector<int> &alignmentPatternCenters_;
  std::vector<ECBlocks*> ecBlocks_;
  int totalCodewords_;
  Version(int versionNumber, std::vector<int> *alignmentPatternCenters, ECBlocks *ecBlocks1, ECBlocks *ecBlocks2,
          ECBlocks *ecBlocks3, ECBlocks *ecBlocks4);

public:
  static unsigned int VERSION_DECODE_INFO[];
  static int N_VERSION_DECODE_INFOS;
  static std::vector<Ref<Version> > VERSIONS;

  ~Version();
  int getVersionNumber() const;
  std::vector<int> &getAlignmentPatternCenters();
  int getTotalCodewords();
  int getDimensionForVersion();
  ECBlocks &getECBlocksForLevel(const ErrorCorrectionLevel &ecLevel) const;
  static Ref<Version> getProvisionalVersionForDimension(int dimension);
  static Ref<Version> getVersionForNumber(int versionNumber);
  static Ref<Version> decodeVersionInformation(unsigned int versionBits);
  Ref<BitMatrix> buildFunctionPattern();
  static int buildVersions();
};
}
}

#endif // ZXING_VERSION_H
