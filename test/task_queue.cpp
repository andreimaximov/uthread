#include <gtest/gtest.h>

#include <uthread/task.hpp>
#include <uthread/task_loop.hpp>
#include <uthread/task_queue.hpp>

namespace uthread {

TEST(TaskQueueTest, ParkAndUnpark) {
  int x = 0;
  TaskQueue queue;

  auto f = [&x, &queue]() {
    ASSERT_EQ(x, 0);
    x++;
    queue.park();
    ASSERT_EQ(x, 2);
    x++;
  };

  auto g = [&x, &queue]() {
    ASSERT_EQ(x, 1);
    x++;
    ASSERT_TRUE(queue.unpark());
    Task::yield();
    ASSERT_EQ(x, 3);
    x++;
  };

  TaskLoop taskLoop;
  taskLoop.addTask(f);
  taskLoop.addTask(g);
  taskLoop.runLoop();

  ASSERT_EQ(x, 4);
}

TEST(TaskQueueTest, UnparkNoTasks) {
  TaskQueue queue;

  auto f = [&queue]() { ASSERT_FALSE(queue.unpark()); };

  TaskLoop taskLoop;
  taskLoop.addTask(f);
  taskLoop.runLoop();
}

}  // namespace uthread
