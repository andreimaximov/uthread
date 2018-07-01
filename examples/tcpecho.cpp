#include <iostream>

#include <gflags/gflags.h>

#include <uthread/uthread.hpp>

using namespace uthread;

DEFINE_uint32(tasks, 1, "The number of worker tasks to spawn.");

DEFINE_uint32(port, 8000, "The TCP port number for the server to listen on.");

namespace {

// A queue of accepted connections which workers work on echo'ing.
MpmcQueue<TcpStream> connections{1};

void work() {
  char buf[1024];

  while (true) {
    TcpStream stream;
    connections.pop(stream);

    while (true) {
      auto readLen = stream.read(buf, sizeof(buf));
      if (readLen == 0 || stream.send(buf, readLen) == 0) {
        // Connection has closed.
        break;
      }
    }
  }
}

void runApp() {
  TcpListener::Options options;
  options.reuseAddr = true;
  TcpListener listener{SocketAddrV4{"127.0.0.1", FLAGS_port}, options};

  std::cout << "Running TCP echo server; Use 'ncat 127.0.0.1 " << FLAGS_port
            << "' to send messages." << std::endl;

  while (true) {
    connections.push(listener.accept());
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A TCP echo server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  TaskLoop taskLoop;
  for (auto t = FLAGS_tasks; t > 0; t--) {
    taskLoop.addTask(work);
  }
  taskLoop.addTask(runApp);
  taskLoop.runLoop();

  return 0;
}
