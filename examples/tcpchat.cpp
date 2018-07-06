#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>

#include <uthread/uthread.hpp>

using namespace uthread;

DEFINE_uint32(port, 8000, "The TCP port number for the server to listen on.");

namespace {

struct User {
  // The name of the connected user.
  std::string name;
  // The log sequence number of the next message to send this user.
  std::size_t seq = 0;
  // The TCP connection for this user.
  TcpStream conn;

  User(std::string name, std::size_t seq, TcpStream conn)
      : name{std::move(name)}, seq{seq}, conn{std::move(conn)} {}
};

struct Room {
  // The log of all messages in the chat room.
  std::vector<std::string> log;
  // The task queue tasks sleep on when waiting for new messages from the log.
  TaskQueue park;
  // The queue of available names. Once the names are exhausted new connections
  // are blocked.
  MpmcQueue<std::string> names;

  Room() {
    names.push("Anonymous Iguana");
    names.push("Anonymous Turtle");
    names.push("Anonymous Bear");
    names.push("Anonymous Whale");
    names.push("Anonymous Shark");
    names.push("Anonymous Elephant");
    names.push("Anonymous Wolf");
    names.push("Anonymous Moose");
  }
};

struct Framer {
  std::string buf;

  void append(const char* buf, std::size_t bufLen) {
    this->buf.append(buf, bufLen);
  }

  // Check if a line has been buffered.
  bool has() {
    auto p = buf.find('\n');
    return (p != std::string::npos);
  }

  // Pop one line from the buffer.
  std::string pop() {
    auto p = buf.find('\n');
    if (p == std::string::npos) {
      return "";
    }
    auto s = buf.substr(0, p);
    buf.erase(0, p + 1);
    return s;
  }
};

Room room;

void post(const User& user, std::string message) {
  static const std::string kWhitespace = " \t\r\n";

  // http://www.toptip.ca/2010/03/trim-leading-or-trailing-white-spaces.html
  auto p = message.find_first_not_of(kWhitespace);
  message.erase(0, p);

  p = message.find_last_not_of(kWhitespace);
  if (p != std::string::npos) {
    message.erase(p + 1);
  }

  // Do not send empty messages.
  if (message.empty()) {
    return;
  }

  std::cerr << user.name + ": " + message << std::endl;
  room.log.push_back(user.name + ": " + message + "\n");
  while (room.park.unpark()) {
  }
}

void work(User user) {
  post(user, "<Joined>");

  auto closed = false;

  // Send the message log to this user.
  Task forwarder{[&user, &closed]() {
    while (!closed) {
      if (user.seq < room.log.size()) {
        const std::string& message = room.log[user.seq];
        if (user.conn.send(message.c_str(), message.size())) {
          user.seq++;
        } else {
          closed = true;
        }
      } else {
        room.park.park();
      }
    }
  }};

  // Append messages from this user to the log.
  Task publisher{[&user, &closed]() {
    Framer f;
    char buf[1024];

    while (!closed) {
      auto readLen = user.conn.read(buf, sizeof(buf));
      if (readLen > 0) {
        f.append(buf, readLen);
        while (f.has()) {
          post(user, f.pop());
        }
      } else {
        closed = true;
        while (room.park.unpark()) {
        }
      }
    }
  }};

  publisher.join();
  forwarder.join();

  // Send a notification that the user has disconnected and reuse the name.
  post(user, "<Left>");
  room.names.push(user.name);
}

void runApp() {
  TcpListener::Options options;
  options.reuseAddr = true;
  TcpListener listener{SocketAddrV4{"127.0.0.1", FLAGS_port}, options};

  std::cout << "Running a chat server; Use 'ncat 127.0.0.1 " << FLAGS_port
            << "' to send messages." << std::endl;

  while (true) {
    // Wait for a name to become available. This is a simple way of limiting the
    // number of concurrent users in our chat room.
    std::string name;
    room.names.pop(name);

    // A name has been allocated, so now we just wait for a connection.
    auto conn = listener.accept();
    User user{std::move(name), room.log.size(), std::move(conn)};
    Task{[user{std::move(user)}]() mutable { work(std::move(user)); }};
  }
}

}  // namespace

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A TCP chat server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  TaskLoop taskLoop;
  taskLoop.addTask(runApp);
  taskLoop.runLoop();

  return 0;
}
