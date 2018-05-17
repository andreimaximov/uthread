#include <uthread/detail/context.hpp>

extern "C" {
void uthreadAsmContextSwap(void* current,
                           const void* target) asm("uthreadAsmContextSwap");

void uthreadAsmContextJump(const void* target) asm("uthreadAsmContextJump");
}

namespace uthread {
namespace detail {

void contextSwap(Context& current, const Context& target) {
  uthreadAsmContextSwap(&current, &target);
}

void contextJump(const Context& target) {
  uthreadAsmContextJump(&target);
}

}  // namespace detail
}  // namespace uthread
