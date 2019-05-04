// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-

#ifndef ZXING_NOT_FOUND_EXCEPTION_H
#define ZXING_NOT_FOUND_EXCEPTION_H

/*
 * Copyright 20011 ZXing authors
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

#include <zxing/ReaderException.h>

namespace zxing {

class NotFoundException : public ReaderException {
public:
  NotFoundException() ZXING_NOEXCEPT;
  NotFoundException(const char *msg) ZXING_NOEXCEPT;
  ~NotFoundException() ZXING_NOEXCEPT;
};

}

#endif // ZXING_NOT_FOUND_EXCEPTION_H
