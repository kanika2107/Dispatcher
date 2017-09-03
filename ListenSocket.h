#pragma once

#include "Dispatcher.h"
#include "Socket.h"

#include <arpa/inet.h>
#include <err.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

constexpr int interval = 3;
template <typename ConnectionSocket,typename ConnectionSocketTimer,typename TimerPolicy>
class ListenSocket : public Socket {
  Dispatcher &m_dispatcher;
public:
  ListenSocket(Dispatcher &dispatcher, uint16_t port) : m_dispatcher(dispatcher) {
    m_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (m_fd < 0) err(2, "socket");
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int res = bind(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (res < 0) err(2, "bind");
    res = listen(m_fd, 16);
    if (res < 0) err(2, "listen");
  }

  HandlerResult HandleRead() override {
    sockaddr_in addr;
    socklen_t size = sizeof(addr);
    int fd = accept4(m_fd, reinterpret_cast<sockaddr*>(&addr), &size, SOCK_NONBLOCK);
    if (fd < 0) {
      if (errno != EAGAIN) warn("accept4");
      return HandlerResult::NORMAL_LISTEN;
    }
    std::unique_ptr<ConnectionSocket> temp(new ConnectionSocket(fd));
    m_dispatcher.register_socket(std::move(temp));
    m_dispatcher.register_policy(fd,std::unique_ptr<TimerPolicy> (new TimerPolicy()));
    m_dispatcher.register_timer(fd,std::unique_ptr<ConnectionSocketTimer> (new ConnectionSocketTimer(time(0)+interval,m_dispatcher.getSocket_ptr(fd))),std::unique_ptr<ConnectionSocketTimer> (new ConnectionSocketTimer(time(0)+interval,m_dispatcher.getSocket_ptr(fd))));
   
    return HandlerResult::NORMAL_LISTEN;
  }

  HandlerResult HandleWrite() override {
    abort();
  }

  bool HasDataForWriting() override {
    return false;
  }
};
