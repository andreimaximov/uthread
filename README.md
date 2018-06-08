# README

*uthread* is a lightweight fiber library I wrote just for fun. It is inspired by [Boost.fiber](https://github.com/boostorg/fiber) and [folly fibers](https://github.com/facebook/folly/tree/master/folly/fibers).

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
- **format:** Runs `clang-format` on the source

A dev VM w/a build environment is provided. Just `vagrant up && vagrant ssh`.

