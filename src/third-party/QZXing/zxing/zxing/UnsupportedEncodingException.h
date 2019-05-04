#ifndef UNSUPPORTEDENCODINGEXCEPTION_H
#define UNSUPPORTEDENCODINGEXCEPTION_H

#include <zxing/Exception.h>

namespace zxing {

class UnsupportedEncodingException : public Exception {
 public:
  UnsupportedEncodingException() ZXING_NOEXCEPT;
  UnsupportedEncodingException(char const* msg) ZXING_NOEXCEPT;
  ~UnsupportedEncodingException() ZXING_NOEXCEPT;
};

}

#endif // UNSUPPORTEDENCODINGEXCEPTION_H
