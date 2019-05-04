// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_DECODEHINTS_H
#define ZXING_DECODEHINTS_H
/*
 *  DecodeHintType.h
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

#include <zxing/BarcodeFormat.h>
#include <zxing/ResultPointCallback.h>

namespace zxing {

typedef unsigned int DecodeHintType;
class DecodeHints;
DecodeHints operator | (DecodeHints const&, DecodeHints const&);

class DecodeHints {
 private:
  DecodeHintType hints;
  Ref<ResultPointCallback> callback;

 public:
  static const DecodeHintType AZTEC_HINT;
  static const DecodeHintType CODABAR_HINT;
  static const DecodeHintType CODE_39_HINT;
  static const DecodeHintType CODE_93_HINT;
  static const DecodeHintType CODE_128_HINT;
  static const DecodeHintType DATA_MATRIX_HINT;
  static const DecodeHintType EAN_8_HINT;
  static const DecodeHintType EAN_13_HINT;
  static const DecodeHintType ITF_HINT;
  static const DecodeHintType MAXICODE_HINT;
  static const DecodeHintType PDF_417_HINT;
  static const DecodeHintType QR_CODE_HINT;
  static const DecodeHintType RSS_14_HINT;
  static const DecodeHintType RSS_EXPANDED_HINT;
  static const DecodeHintType UPC_A_HINT;
  static const DecodeHintType UPC_E_HINT;
  static const DecodeHintType UPC_EAN_EXTENSION_HINT;
  static const DecodeHintType ASSUME_GS1;

  static const DecodeHintType TRYHARDER_HINT;
  static const DecodeHintType CHARACTER_SET;
  // static const DecodeHintType ALLOWED_LENGTHS = 1 << 29;
  // static const DecodeHintType ASSUME_CODE_39_CHECK_DIGIT = 1 << 28;
  // static const DecodeHintType NEED_RESULT_POINT_CALLBACK = 1 << 26;
  
  static const DecodeHints PRODUCT_HINT;
  static const DecodeHints ONED_HINT;
  static const DecodeHints DEFAULT_HINT;

  DecodeHints();
  DecodeHints(const DecodeHintType &init);
  DecodeHints(const DecodeHints &other);

  void addFormat(BarcodeFormat toadd);
  bool containsFormat(BarcodeFormat tocheck) const;
  bool isEmpty() const {return (hints==0);}
  void clear() {hints=0;}
  void setTryHarder(bool toset);
  bool getTryHarder() const;

  void setResultPointCallback(Ref<ResultPointCallback> const&);
  Ref<ResultPointCallback> getResultPointCallback() const;

  DecodeHints& operator =(DecodeHints const &other);

  friend DecodeHints operator| (DecodeHints const&, DecodeHints const&);
};

}

#endif // ZXING_DECODEHINTS_H

