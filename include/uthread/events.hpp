#pragma once

#include <event2/event.h>

namespace uthread {

enum class Events : short {
  Read = EV_READ,
  Write = EV_WRITE,
};

inline Events operator|(Events lhs, Events rhs) {
  return static_cast<Events>(static_cast<short>(lhs) | static_cast<short>(rhs));
}

inline Events operator&(Events lhs, Events rhs) {
  return static_cast<Events>(static_cast<short>(lhs) & static_cast<short>(rhs));
}

}  // namespace uthread
