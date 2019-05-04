// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_MODE_H
#define ZXING_MODE_H

/*
 *  Mode.h
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
#include <zxing/qrcode/Version.h>
#include <zxing/common/Counted.h>

namespace zxing {
namespace qrcode {

class Mode : public Counted
{
private:
  int characterCountBitsForVersions0To9_;
  int characterCountBitsForVersions10To26_;
  int characterCountBitsForVersions27AndHigher_;
  int bits_;
  std::string name_;

  Mode(int cbv0_9, int cbv10_26, int cbv27, int bits, char const* name);

public:
  Mode(const Mode& mode);
  Mode();

  static Mode TERMINATOR;
  static Mode NUMERIC;
  static Mode ALPHANUMERIC;
  static Mode STRUCTURED_APPEND;
  static Mode BYTE;
  static Mode ECI;
  static Mode KANJI;
  static Mode FNC1_FIRST_POSITION;
  static Mode FNC1_SECOND_POSITION;
  static Mode HANZI;

  static Mode& forBits(int bits);
  int getCharacterCountBits(const Version *version) const;
  int getBits() const { return bits_; }

  bool operator==(const Mode& other);
  bool operator!=(const Mode& other);

  std::string getName() const {
      if(name_ == "")
          return "null";
      else
          return name_;
  }
};
}
}

#endif // __MODE_H__
