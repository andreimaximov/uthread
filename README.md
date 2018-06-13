# README

*uthread* is a lightweight fiber library I wrote just for fun. It is inspired by [Boost.fiber](https://github.com/boostorg/fiber) and [folly fibers](https://github.com/facebook/folly/tree/master/folly/fibers).

## API

```cpp
#include <iostream>

#include <uthread/uthread.hpp>

using namespace uthread;

int main() {
  TaskLoop taskLoop;

  taskLoop.addTask([]() {
    Task world;

    Task exclamation{[&world]() {
      world.join();
      std::cout << "!" << std::endl;
    }};

    world = Task{[]() { std::cout << "World"; }};

    std::cout << "Hello... ";
    exclamation.join();
  });

  taskLoop.runLoop();

  return 0;
}
```

We have more examples available [here](examples). See [Building](#building) to see how to compile the examples. Once compiled, you can run the examples from the `build` directory.

## Dependencies

- [libevent](https://github.com/libevent/libevent)
- [benchmark](https://github.com/google/benchmark)
- [gflags](https://github.com/gflags/gflags)
- [googletest](https://github.com/google/googletest)

## Building

You will need [Meson](http://mesonbuild.com/) and [Ninja](https://ninja-build.org/) to build. The steps are:

```
meson build && cd build && ninja
```

Some special targets are provided:

- **install:** Install the library and headers on your system
- **test:** Runs unit tests, don't forget to `meson configure -Db_sanitize=address,undefined`
- **benchmark:** Runs benchmarks, don't forget to `meson configure -Dbuildtype=release`
- **smoke:** Runs [smoke](scripts/smoke.py) tests
- **format:** Runs `clang-format` on the source

A dev VM w/a build environment is provided. Just `vagrant up && vagrant ssh`.

