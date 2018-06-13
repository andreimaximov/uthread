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
