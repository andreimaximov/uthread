#pragma once

#include <cstddef>

namespace uthread {

struct Options {
  enum class Timer { Fast, Precise };

  // The size of the stack for each task/fiber.
  std::size_t stackSize = 16'384;

  // The type of libevent timer to use. Affects the
  // EVENT_BASE_FLAG_PRECISE_TIMER config flag:
  // http://www.wangafu.net/~nickm/libevent-book/Ref2_eventbase.html
  Timer timer = Timer::Fast;
};

}  // namespace uthread
