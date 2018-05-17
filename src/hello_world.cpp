#include <cpp/hello_world.hpp>

#include <iostream>

namespace cpp {

int hello_world(const std::string& name) {
  std::cout << "Hello " << name << "!" << std::endl;
  return 42;
}

}  // namespace cpp
