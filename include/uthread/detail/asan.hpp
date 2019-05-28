#pragma once

// clang
#if defined(__has_feature)
#if ((__has_feature(address_sanitizer)))
#define UTHREAD_ASAN 1
#endif
#endif

// gcc
#if defined(__SANITIZE_ADDRESS__)
#if __SANITIZE_ADDRESS__
#define UTHREAD_ASAN 1
#endif
#endif

namespace uthread {
namespace detail {

#if defined(UTHREAD_ASAN)
constexpr bool kAsan = true;
#else
constexpr bool kAsan = false;
#endif

}  // namespace detail
}  // namespace uthread
