#include <uthread/exception.hpp>

#include <errno.h>
#include <string.h>

namespace uthread {

namespace {

const char* errnoToStr(char* buf, std::size_t bufLen) {
#ifdef _GNU_SOURCE
  return strerror_r(errno, buf, bufLen);
#else
  strerror_r(errno, buf, bufLen);
  return buf;
#endif
}

}  // namespace

Exception::Exception(const char* message) : message_{message} {}

Exception Exception::fromErrNo() {
  Exception ex{nullptr};
  ex.message_ = errnoToStr(ex.buf_, sizeof(ex.buf_));
  return ex;
}

const char* Exception::what() const noexcept {
  return message_;
}

}  // namespace uthread
