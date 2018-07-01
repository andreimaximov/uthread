#include <unistd.h>

#include <gtest/gtest.h>

#include <uthread/detail/task_fd.hpp>
#include <uthread/exception.hpp>

namespace uthread {
namespace detail {

TEST(TaskFdTest, Default) {
  TaskFd fd;
  ASSERT_FALSE(fd);
  ASSERT_EQ(fd.fd, -1);
}

TEST(TaskFdTest, MakeBad) {
  ASSERT_THROW(TaskFd(-1), Exception);
}

TEST(TaskFdTest, Move) {
  int pipefd[2];
  ASSERT_EQ(pipe(pipefd), 0);

  TaskFd r{pipefd[0]};
  ASSERT_TRUE(r);
  ASSERT_EQ(r.fd, pipefd[0]);

  TaskFd w{pipefd[1]};
  ASSERT_TRUE(w);
  ASSERT_EQ(w.fd, pipefd[1]);

  TaskFd x{std::move(r)};
  ASSERT_FALSE(r);
  ASSERT_EQ(r.fd, -1);
  ASSERT_TRUE(x);
  ASSERT_EQ(x.fd, pipefd[0]);

  TaskFd y;
  y = std::move(x);
  ASSERT_FALSE(x);
  ASSERT_EQ(x.fd, -1);
  ASSERT_TRUE(y);
  ASSERT_EQ(y.fd, pipefd[0]);
}

}  // namespace detail
}  // namespace uthread
