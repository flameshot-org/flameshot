// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_BARCODE_FORMAT_H
#define ZXING_BARCODE_FORMAT_H

/*
 *  BarcodeFormat.h
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

namespace zxing {

class BarcodeFormat {
public:
  // if you update the enum, update BarcodeFormat.cpp

  enum Value {
    NONE,
    AZTEC,
    CODABAR,
    CODE_39,
    CODE_93,
    CODE_128,
    DATA_MATRIX,
    EAN_8,
    EAN_13,
    ITF,
    MAXICODE,
    PDF_417,
    QR_CODE,
    RSS_14,
    RSS_EXPANDED,
    UPC_A,
    UPC_E,
    UPC_EAN_EXTENSION,
    ASSUME_GS1
  };

  BarcodeFormat(Value v) : value(v) {}  
  const Value value;
  operator Value () const {return value;}

  static char const* barcodeFormatNames[];
};

}

#endif // ZXING_BARCODE_FORMAT_H
