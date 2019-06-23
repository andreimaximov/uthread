#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <uthread/detail/stack.hpp>
#include <uthread/exception.hpp>
#include <uthread/task.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

using testing::MockFunction;
using testing::Return;
using testing::StrictMock;

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

TEST(TaskTest, Sleep) {
  TaskLoop taskLoop;

  taskLoop.addTask([]() { Task::sleep(std::chrono::milliseconds{200}); });

  auto beginAt = std::chrono::steady_clock::now();
  taskLoop.runLoop();
  auto endAt = std::chrono::steady_clock::now();

  auto runMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(endAt - beginAt)
          .count();
  ASSERT_GE(runMs, 190);
  ASSERT_LE(runMs, 210);
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

TEST(TaskTest, MutableLambda) {
  int x = 0;

  TaskLoop taskLoop;
  taskLoop.addTask([&x, y{x}]() mutable {
    y++;
    x = y;
  });
  taskLoop.runLoop();

  ASSERT_EQ(x, 1);
}

TEST(TaskTest, GuardPages) {
  Options opts;
  opts.stackSize = 16'384;
  opts.useGuardPages = true;

  TaskLoop taskLoop{opts};

  taskLoop.addTask(detail::tryToOverflowStack<>);
  // ASSERT_DEATH(taskLoop.runLoop());
}

TEST(TaskTest, RunInMainContextFromTask) {
  TaskLoop taskLoop;

  StrictMock<MockFunction<void()>> f;
  EXPECT_CALL(f, Call()).Times(1);

  taskLoop.addTask([&f]() {
    Task::runInMainContext([&f]() {
      detail::tryToOverflowStack();
      f.Call();
    });
  });

  taskLoop.runLoop();
}

TEST(TaskTest, RunInMainContextFromTaskRecursive) {
  TaskLoop taskLoop;

  StrictMock<MockFunction<void()>> f1;
  EXPECT_CALL(f1, Call()).Times(1);

  StrictMock<MockFunction<void()>> f2;
  EXPECT_CALL(f2, Call()).Times(1);

  taskLoop.addTask([&f1, &f2]() {
    Task::runInMainContext([&f1, &f2]() {
      detail::tryToOverflowStack();
      f1.Call();

      Task::runInMainContext([&f2]() {
        detail::tryToOverflowStack();
        f2.Call();
      });
    });
  });

  taskLoop.runLoop();
}

TEST(TaskTest, RunInMainContextFromMain) {
  StrictMock<MockFunction<void()>> f;
  EXPECT_CALL(f, Call()).Times(1);

  Task::runInMainContext([&f]() {
    detail::tryToOverflowStack();
    f.Call();
  });
}

TEST(TaskTest, RunInMainContextReturnByValue) {
  TaskLoop taskLoop;

  StrictMock<MockFunction<char()>> f;
  EXPECT_CALL(f, Call()).WillOnce(Return('!'));

  taskLoop.addTask([&f]() {
    char c = Task::runInMainContext(f.AsStdFunction());
    EXPECT_EQ(c, '!');
  });

  taskLoop.runLoop();
}

}  // namespace uthread
