#pragma once

#include <cstddef>

namespace uthread {

struct Options {
  // The size of the stack for each task/fiber.
  std::size_t stackSize = 16'384;
};

}  // namespace uthread
