# README

This is a simple C++ 11 starter project with unit tests, benchmarks, etc. using [Meson](https://mesonbuild.com/) for builds.

## Dependencies

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

