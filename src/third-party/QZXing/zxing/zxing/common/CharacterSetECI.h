// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-

#ifndef ZXING_CHARACTERSET_ECI
#define ZXING_CHARACTERSET_ECI

/*
 * Copyright 2008-2011 ZXing authors
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

#include <map>
#include <zxing/DecodeHints.h>
#include <zxing/common/Counted.h>
#include <vector>

namespace zxing {
namespace common {

class CharacterSetECI : public Counted {
private:
  static std::map<int, CharacterSetECI*> VALUE_TO_ECI;
  static std::map<std::string, CharacterSetECI*> NAME_TO_ECI;
  static std::vector<CharacterSetECI*> ECItables;
  static bool inited;
  static bool init_tables();

  std::vector<int> values_;
  std::vector<std::string> names_;

  CharacterSetECI(const std::vector<int> values, const std::vector<const char*> names);

  static void addCharacterSet(const std::vector<int> value, const std::vector<const char*> encodingNames);

public:
  char const* name() const;
  int getValue() const;

  static CharacterSetECI* getCharacterSetECIByValue(int value);
  static CharacterSetECI* getCharacterSetECIByName(std::string const& name);
  static void removeAllCharacterSets();
};

}
}

#endif // ZXING_CHARACTERSET_ECI

