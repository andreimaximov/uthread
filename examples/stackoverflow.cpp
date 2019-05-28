#include <array>

#include <uthread/uthread.hpp>

using namespace uthread;

int main() {
  Options options;
  options.stackSize = 16'384;
  options.useGuardPages = true;

  TaskLoop taskLoop{options};

  taskLoop.addTask([]() {
    // Use volatile to trick compiler away from optimizing side-effect-less
    // code. This should reliably cause a SIGSEGV.
    std::array<char, 65'536> xs;
    volatile char* p = &xs[0];
    while (p != &xs[0] + xs.size()) {
      *p = '!';
      p++;
    }
  });

  taskLoop.runLoop();

  return 0;
}
