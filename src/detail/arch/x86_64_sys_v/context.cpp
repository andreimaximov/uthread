#include <uthread/detail/context.hpp>

#include <cstdint>

#include <uthread/detail/likely.hpp>
#include <uthread/exception.hpp>

namespace uthread {
namespace detail {

void contextMake(Context& context, void* stack, std::size_t stackSize,
                 void (*f)()) {
  if (UTHREAD_UNLIKELY(stack == nullptr)) {
    throw Exception{"Context: Stack cannot be a nullptr."};
  } else if (UTHREAD_UNLIKELY(f == nullptr)) {
    throw Exception{"Context: Function cannot be a nullptr."};
  } else if (UTHREAD_UNLIKELY(stackSize < kMinStackSize)) {
    stackSize = kMinStackSize;
  }

  // x86-64 stack grows "down".
  auto stackAddr = reinterpret_cast<std::uint64_t>(stack) + stackSize - 1;

  // https://software.intel.com/sites/default/files/article/402129/mpx-linux64-abi.pdf
  // "... the value (%rsp + 8) is always a multiple of 16 (32 or 64) when
  // control is transferred to the function entry point..."
  auto shiftAddr = (stackAddr + 8) % 16;
  stackAddr -= shiftAddr;

  // Setup register to load in the next contextSwap(...).
  context.rbp = stackAddr;
  context.rsp = stackAddr;
  context.rip = reinterpret_cast<std::uint64_t>(f);
}

}  // namespace detail
}  // namespace uthread
