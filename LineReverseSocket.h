#pragma once

#include "Socket.h"

#include <algorithm>
#include <cstring>
#include <err.h>
#include <unistd.h>
#include <iostream>

class LineReverseSocket : public Socket {
  std::string m_read_buffer;
  std::string m_write_buffer;
  static constexpr size_t BUFFER_SIZE = 2048;
public:
  LineReverseSocket(int fd) {
    m_fd = fd;
  }

  //Just for testing
  HandlerResult PrepareHeartbeat()
  {
      std::cout << "Preparing Heartbeat from Timer callback" << std::endl;
      m_write_buffer="full";
      return HandlerResult::NORMAL;
  }

  HandlerResult CloseConnection()
  {

    return HandlerResult::REMOVE;
  }


  HandlerResult HandleRead() override {
    char buffer_alloc[BUFFER_SIZE];
    char *buffer = buffer_alloc;
    ssize_t res = read(m_fd, buffer, sizeof(buffer));
    if (res < 0) {
      if (errno != EAGAIN) warn("read");
      return HandlerResult::NORMAL;
    } else if (res == 0) {
      return HandlerResult::REMOVE;
    } else { // res > 0
      size_t size = res;
      while (true) {
        char *newline = reinterpret_cast<char*>(memchr(buffer, '\n', size));
        if (newline == nullptr) {
          m_read_buffer.append(buffer, size);
          break;
        }
        m_read_buffer.append(buffer, newline);
        flush_buffer();
        size -= newline - buffer + 1;
        buffer = newline + 1;
      }
    }
    return HandlerResult::NORMAL;
  }

  // Called exactly once per newline, buffer does not contain newline
  void flush_buffer() {
    std::string line = std::move(m_read_buffer);
    m_read_buffer.clear();
    std::reverse(line.begin(), line.end());
    m_write_buffer.append(line);
    m_write_buffer.append("\n");
  }

  HandlerResult HandleWrite() override {
    ssize_t res = write(m_fd, m_write_buffer.data(), m_write_buffer.size());
    if (res < 0) {
      if (errno != EAGAIN) warn("write");
      return HandlerResult::NORMAL;
    }
    m_write_buffer.erase(0, res);
    return HandlerResult::NORMAL;
  }

  bool HasDataForWriting() override {
    return !m_write_buffer.empty();
  }
};
