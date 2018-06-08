#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <uthread/detail/task.hpp>
#include <uthread/exception.hpp>

namespace uthread {
namespace detail {

TEST(TaskTest, JumpToTaskAndBack) {
  int x = 0;
  Task::jumpToTask(Task::make([&x]() { x++; }));
  ASSERT_EQ(x, 1);
}

TEST(TaskTest, JumpBetweenTasksAndBack) {
  int x = 0;
  TaskQueue queue;

  auto yield = [&queue]() {
    auto task = queue.pop();
    if (task) {
      Task::swapToTask(std::move(task), queue);
    }
  };

  auto a = Task::make([&x, &yield]() {
    ASSERT_EQ(x, 0);
    yield();
    ASSERT_EQ(x, 1);
    yield();
    ASSERT_EQ(x, 2);
    yield();
  });

  auto b = Task::make([&x, &yield]() {
    x++;
    yield();
    x++;
    yield();
    x++;
  });

  queue.push(std::move(b));
  Task::jumpToTask(std::move(a));
  ASSERT_EQ(x, 3);
}

TEST(TaskTest, PushAndPop) {
  TaskQueue queue;

  ASSERT_FALSE(queue.pop());

  auto a = Task::make([]() {});
  auto b = Task::make([]() {});
  auto ap = a.get();
  auto bp = b.get();

  queue.push(std::move(a));
  queue.push(std::move(b));

  ASSERT_EQ(queue.pop().get(), ap);
  ASSERT_EQ(queue.pop().get(), bp);
  ASSERT_FALSE(queue.pop());
}

TEST(TaskTest, HasTasks) {
  TaskQueue queue;
  ASSERT_FALSE(queue.hasTasks());

  queue.push(Task::make([]() {}));
  ASSERT_TRUE(queue.hasTasks());

  queue.pop();
  ASSERT_FALSE(queue.hasTasks());
}

TEST(TaskTest, DeathOnException) {
  auto a = Task::make([]() { throw Exception{"Halp"}; });
  ASSERT_DEATH(Task::jumpToTask(std::move(a)), "Halp");
}

}  // namespace detail
}  // namespace uthread
