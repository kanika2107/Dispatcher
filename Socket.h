#pragma once

#include <unistd.h>
#include <ctime>

class Socket {
protected:
  Socket() = default;
  int m_fd;
  time_t m_read_timeout;
  time_t m_write_timeout;
public:
  Socket(const Socket &) = delete;
  int fd() const { return m_fd; }

  enum class HandlerResult {
    NORMAL,
    REMOVE,
    NORMAL_LISTEN,        //This is returned by listener sockets to make a distinction b/w listener socket and connection sockets
    NORMAL_CONTINUE,      //This is returned in case of Partial Message read See MessageRecieveSocket.cpp HandleRead() for more details.
  };

  virtual HandlerResult HandleRead() = 0;
  virtual HandlerResult HandleWrite() = 0;
  virtual bool HasDataForWriting() = 0;
  time_t getReadTimeout()
  {
    return m_read_timeout;
  }
  time_t getWriteTimeout()
  {
    return m_write_timeout;
  }
  void setReadTimeout(time_t read_timeout)
  {
    m_read_timeout=read_timeout;
  }
  void setWriteTimeout(time_t write_timeout)
  {
    m_write_timeout=write_timeout;
  }
  virtual ~Socket() {
    close(m_fd);
  }
};
