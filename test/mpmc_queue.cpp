#include <gtest/gtest.h>

#include <functional>

#include <uthread/exception.hpp>
#include <uthread/mpmc_queue.hpp>
#include <uthread/task.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

namespace {

class S {
 public:
  S(std::function<void()> f) : f{f} {}

  S(S&& s) { *this = std::move(s); }

  S& operator=(S&& s) {
    if (s.f) {
      s.f();
    }
    f = s.f;
    return *this;
  }

 private:
  std::function<void()> f;
};

}  // namespace

TEST(MpmcQueueTest, UnboundedPushAndPopFromSingleTask) {
  TaskLoop taskLoop;

  MpmcQueue<int> q;

  taskLoop.addTask([&q]() {
    for (int k = 0; k < 100; k++) {
      q.push(k);
    }

    for (int k = 0; k < 100; k++) {
      int x;
      q.pop(x);
      ASSERT_EQ(x, k);
    }
  });

  taskLoop.runLoop();
}

TEST(MpmcQueueTest, PopWakeManySleepers) {
  TaskLoop taskLoop;

  MpmcQueue<int> q{1};
  int called = 0;
  int returned = 0;

  taskLoop.addTask([&q, &called, &returned]() {
    for (int k = 0; k < 100; k++) {
      Task task{[&q, &called, &returned]() {
        called++;
        q.push(-1);
        returned++;
      }};
    }

    Task::yield();
    ASSERT_EQ(called, 100);
    ASSERT_EQ(returned, 1);

    for (int k = 0; k < 100; k++) {
      int x;
      q.pop(x);
      ASSERT_EQ(x, -1);
    }

    Task::yield();
    ASSERT_EQ(returned, 100);
  });

  taskLoop.runLoop();
}

TEST(MpmcQueueTest, PushWakeManySleepers) {
  TaskLoop taskLoop;

  MpmcQueue<int> q{1};
  int called = 0;
  int returned = 0;

  taskLoop.addTask([&q, &called, &returned]() {
    for (int k = 0; k < 100; k++) {
      Task task{[&q, &called, &returned]() {
        int x;
        called++;
        q.pop(x);
        ASSERT_EQ(x, -1);
        returned++;
      }};
    }

    Task::yield();
    ASSERT_EQ(called, 100);
    ASSERT_EQ(returned, 0);

    for (int k = 0; k < 100; k++) {
      q.push(-1);
    }

    Task::yield();
    ASSERT_EQ(returned, 100);
  });

  taskLoop.runLoop();
}

TEST(MpmcQueue, PushThrows) {
  TaskLoop taskLoop;

  MpmcQueue<S> q{1};

  taskLoop.addTask([&q]() { q.push(S{{}}); });

  taskLoop.addTask([&q]() {
    S s{[]() { throw Exception{""}; }};
    ASSERT_THROW(q.push(std::move(s)), Exception);
  });

  taskLoop.addTask([&q]() { q.push(S{{}}); });

  taskLoop.addTask([&q]() {
    S s{{}};
    q.pop(s);
    q.pop(s);
  });

  taskLoop.runLoop();
}

TEST(MpmcQueue, PopThrows) {
  TaskLoop taskLoop;

  MpmcQueue<S> q{1};

  taskLoop.addTask([&q]() {
    S s{{}};
    ASSERT_THROW(q.pop(s), Exception);
  });

  taskLoop.addTask([&q]() {
    S s{{}};
    q.pop(s);
  });

  taskLoop.addTask([&q]() {
    int moves = 0;
    q.push(S{[moves]() mutable {
      moves++;
      if (moves == 2) {
        throw Exception{""};
      }
    }});
  });

  taskLoop.runLoop();
}

}  // namespace uthread
