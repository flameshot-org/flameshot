#ifndef WRITEREXCEPTION_H
#define WRITEREXCEPTION_H

#include <zxing/Exception.h>

namespace zxing {

class WriterException : public Exception {
 public:
  WriterException() ZXING_NOEXCEPT;
  WriterException(char const* msg) ZXING_NOEXCEPT;
  ~WriterException() ZXING_NOEXCEPT;
};

}

#endif // WRITEREXCEPTION_H
