#ifndef ZXING_READER_H
#define ZXING_READER_H

/*
 *  Reader.h
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

#include <zxing/BinaryBitmap.h>
#include <zxing/Result.h>
#include <zxing/DecodeHints.h>

namespace zxing {

 class Reader : public Counted {
  protected:
   Reader() {}
  public:
   virtual Ref<Result> decode(Ref<BinaryBitmap> image);
   virtual Ref<Result> decode(Ref<BinaryBitmap> image, DecodeHints hints) = 0;
   virtual ~Reader();
};

}

#endif // ZXING_READER_H
