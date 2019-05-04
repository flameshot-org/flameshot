#ifndef ZXING_DATA_BLOCK_DM_H
#define ZXING_DATA_BLOCK_DM_H

/*
 *  DataBlock.h
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

#include <vector>
#include <zxing/common/Counted.h>
#include <zxing/common/Array.h>
#include <zxing/datamatrix/Version.h>

namespace zxing {
namespace datamatrix {

class DataBlock : public Counted {
private:
  int numDataCodewords_;
  ArrayRef<zxing::byte> codewords_;

  DataBlock(int numDataCodewords, ArrayRef<zxing::byte> codewords);

public:  
  static std::vector<Ref<DataBlock> > getDataBlocks(ArrayRef<zxing::byte> rawCodewords, Version *version);

  int getNumDataCodewords();
  ArrayRef<zxing::byte> getCodewords();
};

}
}

#endif // ZXING_DATA_BLOCK_DM_H
