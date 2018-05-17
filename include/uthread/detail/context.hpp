#pragma once

#include <cstddef>

// TODO(amaximov): We should add Solaris and FreeBSD here in the future.
#if defined(__x86_64__) && (defined(__linux__) || defined(__APPLE__))
#include <uthread/detail/arch/x86_64_sys_v/context.hpp>
#else
#error "Sorry, your platform is not supported!"
#endif

namespace uthread {
namespace detail {

// The minimum size of a stack that can be passed to contextMake(...).
constexpr std::size_t kMinStackSize = 256;

// Make a context which executes f() in the next call to contextSwap(...) as
// the target.
//
// You should switch to a different context or terminate the program within f.
// Otherwise behavior upon termination of f is undefined.
void contextMake(Context& context, void* stack, std::size_t stackSize,
                 void (*f)());

// Saves the executing context to current and jumps to another target context.
//
// The target context MUST have been initialized via contextMake(...) or a
// previous call to contextSwap(...) as the "current" context.
void contextSwap(Context& current, const Context& target);

// Jumps to a target context.
//
// You should only use this when leaving the current context permanently.
void contextJump(const Context& target);

}  // namespace detail
}  // namespace uthread
