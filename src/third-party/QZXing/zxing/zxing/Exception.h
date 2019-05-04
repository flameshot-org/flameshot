// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_EXCEPTION_H
#define ZXING_EXCEPTION_H

/*
 *  Exception.h
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

#include <string>
#include <exception>

namespace zxing {

class Exception : public std::exception {
private:
  char const* const message;

public:
  Exception() ZXING_NOEXCEPT;
  Exception(const char* msg) ZXING_NOEXCEPT;
  Exception(Exception const& that) ZXING_NOEXCEPT;
  ~Exception() ZXING_NOEXCEPT;
  char const* what() const ZXING_NOEXCEPT;

private:
  static char const* copy(char const*);
  void deleteMessage();
};

}

#endif // ZXING_EXCEPTION_H
