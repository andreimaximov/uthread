#pragma once

#include <cstddef>

namespace uthread {

struct Options {
  enum class Timer { Fast, Precise };

  // The size of the stack for each task/fiber.
  std::size_t stackSize = 16'384;

  // To enable/disable guard pages. When enabled an additional "guard page" is
  // allocated at the end of each stack. This increases memory usage but helps
  // in debugging by triggering SIGSEGV (cn *nix systems) in case of a stack
  // overflow.
  bool useGuardPages = false;

  // The type of libevent timer to use. Affects the
  // EVENT_BASE_FLAG_PRECISE_TIMER config flag:
  // http://www.wangafu.net/~nickm/libevent-book/Ref2_eventbase.html
  Timer timer = Timer::Fast;
};

}  // namespace uthread
