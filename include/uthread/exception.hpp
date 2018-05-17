#pragma once

#include <exception>

namespace uthread {

class Exception : public std::exception {
 public:
  // Creates an exception with a message. The message is NOT copied! It should
  // be static data to avoid a dangling pointer.
  Exception(const char* message);

  const char* what() const noexcept override;

 private:
  const char* message_ = nullptr;
};

}  // namespace uthread