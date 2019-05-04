// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
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

#include <zxing/common/CharacterSetECI.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/FormatException.h>
#include <cstdlib>

using std::string;
using zxing::IllegalArgumentException;

namespace zxing {
namespace common {

std::map<int, CharacterSetECI*> CharacterSetECI::VALUE_TO_ECI;
std::map<std::string, CharacterSetECI*> CharacterSetECI::NAME_TO_ECI;
std::vector<CharacterSetECI*> CharacterSetECI::ECItables;

bool CharacterSetECI::inited = CharacterSetECI::init_tables();

#define COMMA ,

#define ADD_CHARACTER_SET(VALUES, STRINGS) \
  { int values[] = {VALUES}; \
    const char *strings[] = {STRINGS}; \
    addCharacterSet(std::vector<int>(values, values + sizeof(values) / sizeof(values[0])), \
                    std::vector<const char*>(strings, strings + sizeof(strings) / sizeof(strings[0]))); }

bool CharacterSetECI::init_tables() {
  ADD_CHARACTER_SET(1 COMMA 3, "ISO8859_1" COMMA "ISO-8859-1")
  ADD_CHARACTER_SET(0 COMMA 2, "Cp437");
  ADD_CHARACTER_SET(4, "ISO8859_2" COMMA "ISO-8859-2");
  ADD_CHARACTER_SET(5, "ISO8859_3" COMMA "ISO-8859-3");
  ADD_CHARACTER_SET(6, "ISO8859_4" COMMA "ISO-8859-4");
  ADD_CHARACTER_SET(7, "ISO8859_5" COMMA "ISO-8859-5");
  ADD_CHARACTER_SET(8, "ISO8859_6" COMMA "ISO-8859-6");
  ADD_CHARACTER_SET(9, "ISO8859_7" COMMA "ISO-8859-7");
  ADD_CHARACTER_SET(10, "ISO8859_8" COMMA "ISO-8859-8");
  ADD_CHARACTER_SET(11, "ISO8859_9" COMMA "ISO-8859-9");
  ADD_CHARACTER_SET(12, "ISO8859_10" COMMA "ISO-8859-10");
  ADD_CHARACTER_SET(13, "ISO8859_11" COMMA "ISO-8859-11");
  ADD_CHARACTER_SET(15, "ISO8859_13" COMMA "ISO-8859-13");
  ADD_CHARACTER_SET(16, "ISO8859_14" COMMA "ISO-8859-14");
  ADD_CHARACTER_SET(17, "ISO8859_15" COMMA "ISO-8859-15");
  ADD_CHARACTER_SET(18, "ISO8859_16" COMMA "ISO-8859-16");
  ADD_CHARACTER_SET(20, "SJIS" COMMA "Shift_JIS");
  ADD_CHARACTER_SET(21, "Cp1250" COMMA "windows-1250");
  ADD_CHARACTER_SET(22, "Cp1251" COMMA "windows-1251");
  ADD_CHARACTER_SET(23, "Cp1252" COMMA "windows-1252");
  ADD_CHARACTER_SET(24, "Cp1256" COMMA "windows-1256");
  ADD_CHARACTER_SET(25, "UnicodeBigUnmarked" COMMA "UTF-16BE" COMMA "UnicodeBig");
  ADD_CHARACTER_SET(26, "UTF8" COMMA "UTF-8");
  ADD_CHARACTER_SET(27 COMMA 170, "ASCII" COMMA "US-ASCII");
  ADD_CHARACTER_SET(28, "Big5");
  ADD_CHARACTER_SET(29, "GB18030" COMMA "GB2312" COMMA "EUC_CN" COMMA "GBK");
  ADD_CHARACTER_SET(30, "EUC_KR" COMMA "EUC-KR");

  std::atexit(removeAllCharacterSets);

  return true;
}

CharacterSetECI::CharacterSetECI(const std::vector<int> values, const std::vector<const char*> names)
  : Counted(), values_(values)
{
    for(size_t i=0; i<names.size(); i++)
        names_.push_back(std::string(names[i]));
}

char const* CharacterSetECI::name() const {
  return names_[0].c_str();
}

int CharacterSetECI::getValue() const {
  return values_[0];
}

void CharacterSetECI::addCharacterSet(const std::vector<int> values, const std::vector<const char*> names) {
  CharacterSetECI* charSet = new CharacterSetECI(values, names);
  for(size_t i=0; i<values.size(); i++) {
    VALUE_TO_ECI[values[i]] = charSet;
  }
  for(size_t i=0; i<names.size(); i++) {
    NAME_TO_ECI[std::string(names[i])] = charSet;
  }

  ECItables.push_back(charSet);
}

CharacterSetECI* CharacterSetECI::getCharacterSetECIByValue(int value) {
  if (value < 0 || value >= 900) {
    throw FormatException();
  }
  return VALUE_TO_ECI[value];
}

CharacterSetECI* CharacterSetECI::getCharacterSetECIByName(string const& name) {
  return NAME_TO_ECI[name];
}

void CharacterSetECI::removeAllCharacterSets()
{
    VALUE_TO_ECI.clear();
    NAME_TO_ECI.clear();

    for (std::vector<CharacterSetECI*>::iterator i=ECItables.begin();i!=ECItables.end();i++)
    {
        delete *i;
    }
    ECItables.clear();

    inited=false;
 }

}
}
