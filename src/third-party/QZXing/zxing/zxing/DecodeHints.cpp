// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  DecodeHintType.cpp
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

#include <zxing/DecodeHints.h>
#include <zxing/common/IllegalArgumentException.h>
#include <qglobal.h>

using zxing::Ref;
using zxing::ResultPointCallback;
using zxing::DecodeHints;

// VC++
using zxing::BarcodeFormat;

const zxing::DecodeHintType DecodeHints::AZTEC_HINT = 1 << BarcodeFormat::AZTEC;
const zxing::DecodeHintType DecodeHints::CODABAR_HINT = 1 << BarcodeFormat::CODABAR;
const zxing::DecodeHintType DecodeHints::CODE_39_HINT = 1 << BarcodeFormat::CODE_39;
const zxing::DecodeHintType DecodeHints::CODE_93_HINT = 1 << BarcodeFormat::CODE_93;
const zxing::DecodeHintType DecodeHints::CODE_128_HINT = 1 << BarcodeFormat::CODE_128;
const zxing::DecodeHintType DecodeHints::DATA_MATRIX_HINT = 1 << BarcodeFormat::DATA_MATRIX;
const zxing::DecodeHintType DecodeHints::EAN_8_HINT = 1 << BarcodeFormat::EAN_8;
const zxing::DecodeHintType DecodeHints::EAN_13_HINT = 1 << BarcodeFormat::EAN_13;
const zxing::DecodeHintType DecodeHints::ITF_HINT = 1 << BarcodeFormat::ITF;
const zxing::DecodeHintType DecodeHints::MAXICODE_HINT = 1 << BarcodeFormat::MAXICODE;
const zxing::DecodeHintType DecodeHints::PDF_417_HINT = 1 << BarcodeFormat::PDF_417;
const zxing::DecodeHintType DecodeHints::QR_CODE_HINT = 1 << BarcodeFormat::QR_CODE;
const zxing::DecodeHintType DecodeHints::RSS_14_HINT = 1 << BarcodeFormat::RSS_14;
const zxing::DecodeHintType DecodeHints::RSS_EXPANDED_HINT = 1 << BarcodeFormat::RSS_EXPANDED;
const zxing::DecodeHintType DecodeHints::UPC_A_HINT = 1 << BarcodeFormat::UPC_A;
const zxing::DecodeHintType DecodeHints::UPC_E_HINT = 1 << BarcodeFormat::UPC_E;
const zxing::DecodeHintType DecodeHints::UPC_EAN_EXTENSION_HINT = 1 << BarcodeFormat::UPC_EAN_EXTENSION;
const zxing::DecodeHintType DecodeHints::ASSUME_GS1 = 1 << BarcodeFormat::ASSUME_GS1;
const zxing::DecodeHintType DecodeHints::TRYHARDER_HINT = 1 << 31;
const zxing::DecodeHintType DecodeHints::CHARACTER_SET = 1 << 30;

const zxing::DecodeHints DecodeHints::PRODUCT_HINT(
  DecodeHints::UPC_A_HINT |
  DecodeHints::UPC_E_HINT |
  DecodeHints::EAN_13_HINT |
  DecodeHints::EAN_8_HINT |
  DecodeHints::RSS_14_HINT
  );

const zxing::DecodeHints DecodeHints::ONED_HINT(
  DecodeHints::CODE_39_HINT |
  DecodeHints::CODE_93_HINT |
  DecodeHints::CODE_128_HINT |
  DecodeHints::ITF_HINT |
  DecodeHints::CODABAR_HINT |
  DecodeHints::PRODUCT_HINT
  );

const zxing::DecodeHints DecodeHints::DEFAULT_HINT(
  DecodeHints::ONED_HINT |
  DecodeHints::QR_CODE_HINT |
  DecodeHints::DATA_MATRIX_HINT |
  DecodeHints::AZTEC_HINT |
  DecodeHints::PDF_417_HINT
  );

DecodeHints::DecodeHints() {
  hints = 0;
}

DecodeHints::DecodeHints(const zxing::DecodeHintType &init) {
    hints = init;
}

DecodeHints::DecodeHints(const DecodeHints &other) {
    hints = other.hints;
    callback = other.callback;
}

void DecodeHints::addFormat(BarcodeFormat toadd) {
  switch (toadd) {
  case BarcodeFormat::AZTEC: hints |= AZTEC_HINT; break;
  case BarcodeFormat::CODABAR: hints |= CODABAR_HINT; break;
  case BarcodeFormat::CODE_39: hints |= CODE_39_HINT; break;
  case BarcodeFormat::CODE_93: hints |= CODE_93_HINT; break;
  case BarcodeFormat::CODE_128: hints |= CODE_128_HINT; break;
  case BarcodeFormat::DATA_MATRIX: hints |= DATA_MATRIX_HINT; break;
  case BarcodeFormat::EAN_8: hints |= EAN_8_HINT; break;
  case BarcodeFormat::EAN_13: hints |= EAN_13_HINT; break;
  case BarcodeFormat::ITF: hints |= ITF_HINT; break;
  case BarcodeFormat::MAXICODE: hints |= MAXICODE_HINT; break;
  case BarcodeFormat::PDF_417: hints |= PDF_417_HINT; break;
  case BarcodeFormat::QR_CODE: hints |= QR_CODE_HINT; break;
  case BarcodeFormat::RSS_14: hints |= RSS_14_HINT; break;
  case BarcodeFormat::RSS_EXPANDED: hints |= RSS_EXPANDED_HINT; break;
  case BarcodeFormat::UPC_A: hints |= UPC_A_HINT; break;
  case BarcodeFormat::UPC_E: hints |= UPC_E_HINT; break;
  case BarcodeFormat::UPC_EAN_EXTENSION: hints |= UPC_EAN_EXTENSION_HINT; break;
  case BarcodeFormat::ASSUME_GS1: hints |= ASSUME_GS1; break;
  default: throw IllegalArgumentException("Unrecognizd barcode format");
  }
}

bool DecodeHints::containsFormat(BarcodeFormat tocheck) const {
  DecodeHintType checkAgainst = 0;
  switch (tocheck) {
  case BarcodeFormat::AZTEC: checkAgainst |= AZTEC_HINT; break;
  case BarcodeFormat::CODABAR: checkAgainst |= CODABAR_HINT; break;
  case BarcodeFormat::CODE_39: checkAgainst |= CODE_39_HINT; break;
  case BarcodeFormat::CODE_93: checkAgainst |= CODE_93_HINT; break;
  case BarcodeFormat::CODE_128: checkAgainst |= CODE_128_HINT; break;
  case BarcodeFormat::DATA_MATRIX: checkAgainst |= DATA_MATRIX_HINT; break;
  case BarcodeFormat::EAN_8: checkAgainst |= EAN_8_HINT; break;
  case BarcodeFormat::EAN_13: checkAgainst |= EAN_13_HINT; break;
  case BarcodeFormat::ITF: checkAgainst |= ITF_HINT; break;
  case BarcodeFormat::MAXICODE: checkAgainst |= MAXICODE_HINT; break;
  case BarcodeFormat::PDF_417: checkAgainst |= PDF_417_HINT; break;
  case BarcodeFormat::QR_CODE: checkAgainst |= QR_CODE_HINT; break;
  case BarcodeFormat::RSS_14: checkAgainst |= RSS_14_HINT; break;
  case BarcodeFormat::RSS_EXPANDED: checkAgainst |= RSS_EXPANDED_HINT; break;
  case BarcodeFormat::UPC_A: checkAgainst |= UPC_A_HINT; break;
  case BarcodeFormat::UPC_E: checkAgainst |= UPC_E_HINT; break;
  case BarcodeFormat::UPC_EAN_EXTENSION: checkAgainst |= UPC_EAN_EXTENSION_HINT; break;
  case BarcodeFormat::ASSUME_GS1: checkAgainst |= ASSUME_GS1; break;
  default: throw IllegalArgumentException("Unrecognizd barcode format");
  }
  return (hints & checkAgainst) != 0;
}

void DecodeHints::setTryHarder(bool toset) {
  if (toset) {
    hints |= TRYHARDER_HINT;
  } else {
    hints &= ~TRYHARDER_HINT;
  }
}

bool DecodeHints::getTryHarder() const {
  return (hints & TRYHARDER_HINT) != 0;
}

void DecodeHints::setResultPointCallback(Ref<ResultPointCallback> const& _callback) {
  callback = _callback;
}

Ref<ResultPointCallback> DecodeHints::getResultPointCallback() const {
    return callback;
}

zxing::DecodeHints &zxing::DecodeHints::operator =(const zxing::DecodeHints &other)
{
    hints = other.hints;
    callback = other.callback;
    return *this;
}

zxing::DecodeHints zxing::operator | (DecodeHints const& l, DecodeHints const& r) {
  DecodeHints result (l);
  result.hints |= r.hints;
  if (!result.callback) {
    result.callback = r.callback;
  }
  return result;
}
