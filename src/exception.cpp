#include <uthread/exception.hpp>

namespace uthread {

Exception::Exception(const char* message) : message_{message} {}

const char* Exception::what() const noexcept {
  return message_;
}

}  // namespace uthread