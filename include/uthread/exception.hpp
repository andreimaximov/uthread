#pragma once

#include <exception>

namespace uthread {

class Exception : public std::exception {
 public:
  // Creates an exception with a message. The message is NOT copied! It should
  // be static data to avoid a dangling pointer.
  Exception(const char* message);

  // Return an exception with a message based on errno.
  static Exception fromErrNo();

  const char* what() const noexcept override;

 private:
  const char* message_ = nullptr;
  char buf_[64];
};

}  // namespace uthread
