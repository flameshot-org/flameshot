#ifndef ZXING_RESULT_H
#define ZXING_RESULT_H

/*
 *  Result.h
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

#include <string>
#include <zxing/common/Array.h>
#include <zxing/common/Counted.h>
#include <zxing/common/Str.h>
#include <zxing/common/Types.h>
#include <zxing/ResultPoint.h>
#include <zxing/BarcodeFormat.h>

namespace zxing {

class Result : public Counted {
private:
  Ref<String> text_;
  ArrayRef<zxing::byte> rawBytes_;
  ArrayRef< Ref<ResultPoint> > resultPoints_;
  BarcodeFormat format_;
  std::string charSet_;

public:
  Result(Ref<String> text,
         ArrayRef<zxing::byte> rawBytes,
         ArrayRef< Ref<ResultPoint> > resultPoints,
         BarcodeFormat format, std::string charSet = "");
  ~Result();
  Ref<String> getText();
  ArrayRef<zxing::byte> getRawBytes();
  ArrayRef< Ref<ResultPoint> > const& getResultPoints() const;
  ArrayRef< Ref<ResultPoint> >& getResultPoints();
  BarcodeFormat getBarcodeFormat() const;
  std::string getCharSet() const;

  friend std::ostream& operator<<(std::ostream &out, Result& result);
};

}
#endif // ZXING_RESULT_H
