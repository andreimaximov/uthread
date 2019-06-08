#include <vector>

#include <benchmark/benchmark.h>

#include <uthread/exception.hpp>
#include <uthread/task_loop.hpp>
#include <uthread/tcp_listener.hpp>
#include <uthread/tcp_stream.hpp>

namespace uthread {

// This benchmarks the estimated cost of sending 128 mb of data over localhost
// via TCP. This is to make sure the cost of async IO notifications and
// associated task context switching is not excessively expensive.
static void benchTcpStream(benchmark::State& state) {
  constexpr std::size_t kBufSize = 1024;
  constexpr std::size_t kBufNum = 1024 * 128;

  while (state.KeepRunning()) {
    TaskLoop taskLoop;

    taskLoop.addTask([]() {
      TcpListener::Options options;
      options.reuseAddr = true;

      TcpListener listener{SocketAddrV4{"127.0.0.1", 9876}, options};

      auto stream = listener.accept();
      std::vector<char> buf(kBufSize);

      for (std::size_t k = 0; k < kBufNum; k++) {
        if (stream.send(buf.data(), buf.size()) != buf.size()) {
          throw Exception{"Client closed the connection."};
        }
      }
    });

    taskLoop.addTask([]() {
      TcpStream stream{SocketAddrV4{"127.0.0.1", 9876}};
      std::vector<char> buf(kBufSize);
      std::size_t bytes = 0;

      while (auto n = stream.read(buf.data(), buf.size())) {
        bytes += n;
      }

      if (bytes != kBufSize * kBufNum) {
        throw Exception{"Server closed the connection."};
      }
    });

    taskLoop.runLoop();
  }
}

BENCHMARK(benchTcpStream)->Unit(benchmark::kMillisecond);

}  // namespace uthread
