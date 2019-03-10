# README

*uthread* is a lightweight fiber library I wrote just for fun. It was inspired by [Boost.fiber](https://github.com/boostorg/fiber) and [folly fibers](https://github.com/facebook/folly/tree/master/folly/fibers) and can achieve **30 million task context switches/second**.

[![Build Status](https://travis-ci.com/andreimaximov/uthread.svg?branch=master)](https://travis-ci.com/andreimaximov/uthread)

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

## Design

*uthread* is designed around an N:1 (task:thread) model. A [TaskLoop](include/uthread/task_loop.hpp) is responsible for multiplexing a set of tasks on top of a thread. This includes sleeping and waking threads waiting on asynchronous IO notifications courtesy of [libevent](https://github.com/libevent). The [TaskQueue](include/uthread/task_queue.hpp) forms the basis of building task scheduling and synchronization primitives such as [MpmcQueue](include/uthread/mpmc_queue.hpp).

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

## Examples

- [test/task.cpp](test/task.cpp) for using the Task API
- [test/task_queue.cpp](test/task_queue.cpp) for using the TaskQueue API
- [examples/tcpecho.cpp](examples/tcpecho.cpp) for a TCP echo server
- [examples/tcpchat.cpp](examples/tcpchat.cpp) for a TCP chat server

## Reading

The following are good reads on memory models, compiler/hardware instruction reordering, etc that are important to understand in concurrent environments. *uthread* multiplexes tasks on top of a single thread so most of this is not immediately relevant but still useful for understanding what kind of compiler optimizations might occur around context switches. Despite *thinking* I understand this stuff, I wouldn't be surprised if *uthreads* has bugs related to these topics :)

- [Memory Ordering at Compile Time](http://preshing.com/20120625/memory-ordering-at-compile-time/)
- [Memory Reordering Caught in the Act](http://preshing.com/20120515/memory-reordering-caught-in-the-act/)
- [Is memory reordering visible to other threads on a uniprocessor?](https://stackoverflow.com/questions/14182066/is-memory-reordering-visible-to-other-threads-on-a-uniprocessor)

