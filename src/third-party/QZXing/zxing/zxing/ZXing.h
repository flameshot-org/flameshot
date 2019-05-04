// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2013 ZXing authors All rights reserved.
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
#ifndef ZXING_H
#define ZXING_H

#define ZXING_ARRAY_LEN(v) ((int)(sizeof(v)/sizeof(v[0])))
#define ZX_LOG_DIGITS(digits) \
    ((digits == 8) ? 3 : \
     ((digits == 16) ? 4 : \
      ((digits == 32) ? 5 : \
       ((digits == 64) ? 6 : \
        ((digits == 128) ? 7 : \
         (-1))))))

#ifndef ZXING_DEBUG
#define ZXING_DEBUG 0
#endif

#include <limits>
#include "common/Types.h"

#if defined(_WIN32) || defined(_WIN64)

#include <float.h>
#include <cmath>

namespace zxing {
inline bool isnan_z(float v) {return std::isnan(v) != 0;}
inline bool isnan_z(double v) {return std::isnan(v) != 0;}
}

#elif (__cplusplus >= 201103L)
#include <cmath>
namespace zxing {
inline bool isnan_z(float v) {
    return std::isnan(v);
}
inline bool isnan_z(double v) {
    return std::isnan(v);
}
}
#elif(__STDC_VERSION__ >= 199901L)
#include <math.h>
namespace zxing {
inline bool isnan_z(float v) {
    return isnan(v);
}
inline bool isnan_z(double v) {
    return isnan(v);
}
}
#else
namespace zxing {
inline bool isnan_z(float v) {
    volatile float d = v;
    return d != d;
}
inline bool isnan_z(double v) {
    volatile double d = v;
    return d != d;
}
}
#endif

namespace zxing {
	inline float nan() {return std::numeric_limits<float>::quiet_NaN();}
}

#if ZXING_DEBUG

#include <iostream>
#include <string>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::string;
using std::ostream;

#if ZXING_DEBUG_TIMER

#include <sys/time.h>

namespace zxing {

class DebugTimer {
public:
  DebugTimer(char const* string_) : chars(string_) {
    gettimeofday(&start, 0);
  }

  DebugTimer(std::string const& string_) : chars(0), string(string_) {
    gettimeofday(&start, 0);
  }

  void mark(char const* string) {
    struct timeval end;
    gettimeofday(&end, 0);
    int diff =
      (end.tv_sec - start.tv_sec)*1000*1000+(end.tv_usec - start.tv_usec);
    
    cerr << diff << " " << string << '\n';
  }

  void mark(std::string string) {
    mark(string.c_str());
  }

  ~DebugTimer() {
    if (chars) {
      mark(chars);
    } else {
      mark(string.c_str());
    }
  }

private:
  char const* const chars;
  std::string string;
  struct timeval start;
};

}

#define ZXING_TIME(string) DebugTimer __timer__ (string)
#define ZXING_TIME_MARK(string) __timer__.mark(string)

#endif

#endif // ZXING_DEBUG

#ifndef ZXING_TIME
#define ZXING_TIME(string) (void)0
#endif
#ifndef ZXING_TIME_MARK
#define ZXING_TIME_MARK(string) (void)0
#endif

#ifndef ZXING_NULLPTR
#if __cplusplus >= 201103L
   #define ZXING_NULLPTR nullptr
#else
   #define ZXING_NULLPTR NULL
#endif
#endif // ZXING_NULLPTR

#if __cplusplus >= 201103L
   #define ZXING_NOEXCEPT noexcept
#else
   #define ZXING_NOEXCEPT throw()
#endif

#endif
