#include <gtest/gtest.h>

#include <uthread/exception.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

TEST(TaskLoopTest, RunLoop) {
  TaskLoop taskLoop;
  taskLoop.addTask(
      [&taskLoop]() { ASSERT_THROW(taskLoop.runLoop(), Exception); });
  taskLoop.runLoop();
}

}  // namespace uthread
