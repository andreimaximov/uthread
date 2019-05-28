#include <uthread/detail/stack.hpp>

#include <uthread/detail/asan.hpp>
#include <uthread/exception.hpp>

#include <sys/mman.h>
#include <unistd.h>

namespace uthread {
namespace detail {

std::size_t getSysPageSize() {
  static const std::size_t kPageSize = []() {
    auto pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize == -1) {
      throw Exception::fromErrNo();
    }
    return pageSize;
  }();

  return kPageSize;
}

std::shared_ptr<char> makeStack(std::size_t stackSize, bool useGuardPages) {
  // Let ASAN do it's thing if enabled.
  if (kAsan) {
    return std::shared_ptr<char>{new char[stackSize],
                                 std::default_delete<char[]>{}};
  }

  std::size_t pageSize = 0;

  if (useGuardPages) {
    // > If addr is NULL, then the kernel chooses the (page-aligned) address
    //   at which to create the mapping;
    //
    // - http://man7.org/linux/man-pages//man2/munmap.2.html
    //
    // We can use this to be efficient and get at most one extra guard page.
    pageSize = getSysPageSize();
    if (auto chunk = stackSize % pageSize) {
      stackSize += (pageSize - chunk);
    }
    stackSize += pageSize;
  }

  auto p = mmap(0, stackSize, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (p == MAP_FAILED) {
    throw Exception::fromErrNo();
  }

  auto cp = static_cast<char*>(p);
  auto sp =
      std::shared_ptr<char>{cp, [stackSize](char* p) { munmap(p, stackSize); }};

  if (useGuardPages &&
      mprotect(cp + stackSize - pageSize, pageSize, PROT_NONE) == -1) {
    throw Exception::fromErrNo();
  }

  return sp;
}

}  // namespace detail
}  // namespace uthread
