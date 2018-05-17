#include <gflags/gflags.h>

#include <cpp/hello_world.hpp>

DEFINE_string(name, "", "What is your name?");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  cpp::hello_world(FLAGS_name);
  return 0;
}
