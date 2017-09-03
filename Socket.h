#pragma once

#include <unistd.h>

class Socket {
protected:
  Socket() = default;
  int m_fd;
public:
  Socket(const Socket &) = delete;
  int fd() const { return m_fd; }

  enum class HandlerResult {
    NORMAL,
    REMOVE,
    NORMAL_LISTEN,
  };

  // Returning "false" means please remove me
  virtual HandlerResult HandleRead() = 0;
  virtual HandlerResult HandleWrite() = 0;
  virtual bool HasDataForWriting() = 0;
  virtual ~Socket() {
    close(m_fd);
  }
};
