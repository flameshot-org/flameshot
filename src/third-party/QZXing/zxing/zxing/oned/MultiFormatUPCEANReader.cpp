// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  MultiFormatUPCEANReader.cpp
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

#include <zxing/ZXing.h>
#include <zxing/oned/MultiFormatUPCEANReader.h>
#include <zxing/oned/EAN13Reader.h>
#include <zxing/oned/EAN8Reader.h>
#include <zxing/oned/UPCEReader.h>
#include <zxing/oned/UPCAReader.h>
#include <zxing/oned/OneDResultPoint.h>
#include <zxing/common/Array.h>
#include <zxing/ReaderException.h>
#include <zxing/NotFoundException.h>
#include <math.h>

using zxing::NotFoundException;
using zxing::Ref;
using zxing::Result;
using zxing::oned::MultiFormatUPCEANReader;
    
// VC++
using zxing::DecodeHints;
using zxing::BitArray;

MultiFormatUPCEANReader::MultiFormatUPCEANReader(DecodeHints hints) : readers() {
  if (hints.containsFormat(BarcodeFormat::EAN_13)) {
    readers.push_back(Ref<UPCEANReader>(new EAN13Reader()));
  } else if (hints.containsFormat(BarcodeFormat::UPC_A)) {
    readers.push_back(Ref<UPCEANReader>(new UPCAReader()));
  }
  if (hints.containsFormat(BarcodeFormat::EAN_8)) {
    readers.push_back(Ref<UPCEANReader>(new EAN8Reader()));
  }
  if (hints.containsFormat(BarcodeFormat::UPC_E)) {
    readers.push_back(Ref<UPCEANReader>(new UPCEReader()));
  }
  if (readers.size() == 0) {
    readers.push_back(Ref<UPCEANReader>(new EAN13Reader()));
    // UPC-A is covered by EAN-13
    readers.push_back(Ref<UPCEANReader>(new EAN8Reader()));
    readers.push_back(Ref<UPCEANReader>(new UPCEReader()));
  }
}

#include <typeinfo>

Ref<Result> MultiFormatUPCEANReader::decodeRow(int rowNumber, Ref<BitArray> row, zxing::DecodeHints /*hints*/) {
  // Compute this location once and reuse it on multiple implementations
  UPCEANReader::Range startGuardPattern = UPCEANReader::findStartGuardPattern(row);
  for (int i = 0, e = int(readers.size()); i < e; i++) {
    Ref<UPCEANReader> reader = readers[i];
    Ref<Result> result;
    try {
      result = reader->decodeRow(rowNumber, row, startGuardPattern);
    } catch (ReaderException const& ignored) {
      (void)ignored;
      continue;
    }

    //added this because reader->decodeRow returns null if row is null
    //TODO: investigate why the execution reaches here with empty row.
    if(result.empty())
        continue;

    // Special case: a 12-digit code encoded in UPC-A is identical
    // to a "0" followed by those 12 digits encoded as EAN-13. Each
    // will recognize such a code, UPC-A as a 12-digit string and
    // EAN-13 as a 13-digit string starting with "0".  Individually
    // these are correct and their readers will both read such a
    // code and correctly call it EAN-13, or UPC-A, respectively.
    //
    // In this case, if we've been looking for both types, we'd like
    // to call it a UPC-A code. But for efficiency we only run the
    // EAN-13 decoder to also read UPC-A. So we special case it
    // here, and convert an EAN-13 result to a UPC-A result if
    // appropriate.
    bool ean13MayBeUPCA =
      result->getBarcodeFormat() == BarcodeFormat::EAN_13 &&
      result->getText()->charAt(0) == '0';

    // Note: doesn't match Java which uses hints

    bool canReturnUPCA = true;

    if (ean13MayBeUPCA && canReturnUPCA) {
      // Transfer the metdata across
      Ref<Result> resultUPCA (new Result(result->getText()->substring(1),
                                         result->getRawBytes(),
                                         result->getResultPoints(),
                                         BarcodeFormat::UPC_A));
      // needs java metadata stuff
      return resultUPCA;
    }
    return result;
  }

  throw NotFoundException();
}
