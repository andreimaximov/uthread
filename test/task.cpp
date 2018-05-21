#include <gtest/gtest.h>

#include <uthread/exception.hpp>
#include <uthread/task.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

TEST(TaskTest, RunSingleTask) {
  int x = 0;

  auto f = [&x]() {
    ASSERT_EQ(x, 0);
    x++;
  };

  TaskLoop taskLoop;
  taskLoop.addTask(f);
  taskLoop.runLoop();

  ASSERT_EQ(x, 1);
}

TEST(TaskTest, RunTwoYieldingTasks) {
  int x = 0;

  auto f = [&x]() {
    ASSERT_EQ(x, 0);
    Task::yield();
    ASSERT_EQ(x, 1);
    Task::yield();
    ASSERT_EQ(x, 2);
  };

  auto g = [&x]() {
    x++;
    Task::yield();
    x++;
    Task::yield();
  };

  TaskLoop taskLoop;
  taskLoop.addTask(f);
  taskLoop.addTask(g);
  taskLoop.runLoop();

  ASSERT_EQ(x, 2);
}

TEST(TaskTest, JoinOnRunningTask) {
  int x = 0;

  auto f = [&x]() {
    ASSERT_EQ(x, 0);
    Task::yield();
    ASSERT_EQ(x, 0);
    x++;
  };

  auto g = [&x, &f]() {
    ASSERT_EQ(x, 0);
    Task ftask{f};
    ftask.join();
    ASSERT_EQ(x, 1);
    x++;
  };

  TaskLoop taskLoop;
  taskLoop.addTask(g);
  taskLoop.runLoop();

  ASSERT_EQ(x, 2);
}

TEST(TaskTest, JoinOnFinishedTask) {
  int x = 0;

  auto f = [&x]() {
    ASSERT_EQ(x, 0);
    x++;
  };

  auto g = [&x, &f]() {
    ASSERT_EQ(x, 0);
    Task ftask{f};
    while (x != 1) {
      Task::yield();
    }
    ftask.join();
    x++;
  };

  TaskLoop taskLoop;
  taskLoop.addTask(g);
  taskLoop.runLoop();

  ASSERT_EQ(x, 2);
}

TEST(TaskTest, NextTaskOnFinish) {
  int x = 0;

  auto f = [&x]() {
    ASSERT_EQ(x, 0);
    x++;
  };

  auto g = [&x]() {
    ASSERT_EQ(x, 1);
    x++;
  };

  TaskLoop taskLoop;
  taskLoop.addTask(f);
  taskLoop.addTask(g);
  taskLoop.runLoop();

  ASSERT_EQ(x, 2);
}

TEST(TaskTest, CleanupOnFinish) {
  std::shared_ptr<int> x{new int{0}};
  ASSERT_EQ(x.use_count(), 1);

  auto f = [x]() { Task::yield(); };

  ASSERT_EQ(x.use_count(), 2);

  TaskLoop taskLoop;
  taskLoop.addTask(f);

  ASSERT_EQ(x.use_count(), 3);

  taskLoop.runLoop();

  ASSERT_EQ(x.use_count(), 2);
}

TEST(TaskTest, ExceptionOnDeadlock) {
  Task a;
  Task b;

  TaskLoop taskLoop;
  taskLoop.addTask([&a, &b]() {
    a = Task{[&b]() { b.join(); }};
    b = Task{[&a]() { ASSERT_THROW(a.join(), Exception); }};
  });
  taskLoop.runLoop();
}

}  // namespace uthread
