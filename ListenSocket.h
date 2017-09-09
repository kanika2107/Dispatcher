#pragma once

#include "Dispatcher.h"
#include "Socket.h"

#include <arpa/inet.h>
#include <err.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>

template <typename ConnectionSocket,typename ConnectionSocketTimer,typename TimerPolicy>
class ListenSocket : public Socket {
  Dispatcher &m_dispatcher;
  OrderBookHelper &m_order_book_helper;
  TimerPolicy timer_policy;
  std::map<uint64_t,Session*>& m_sessions; //map of sessions Key:Client token Value:Pointer to the Connection Socket corresponding to this session

public:
  //SOCKET BIND LISTEN
  ListenSocket(Dispatcher &dispatcher, uint16_t port,std::map<uint64_t,Session*>& sessions,OrderBookHelper &order_book_helper) : m_dispatcher(dispatcher),m_sessions(sessions),m_order_book_helper(order_book_helper) {
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
    m_dispatcher.register_socket(std::unique_ptr<ConnectionSocket> (new ConnectionSocket(fd,m_sessions,time(0)+timer_policy.interval,time(0)+timer_policy.interval,m_order_book_helper)));

    m_dispatcher.register_policy(fd,std::unique_ptr<TimerPolicy> (new TimerPolicy()));

    //Registration of timer for this connection socket.Pass the pointer to this connection socket to the Timer constructor since we need to bind it for callbacks
    m_dispatcher.register_timer(fd,std::unique_ptr<ConnectionSocketTimer> (new ConnectionSocketTimer(time(0)+timer_policy.interval,m_dispatcher.getSocket_ptr(fd),fd,'R')),std::unique_ptr<ConnectionSocketTimer> (new ConnectionSocketTimer(time(0)+timer_policy.interval,m_dispatcher.getSocket_ptr(fd),fd,'W')));

    return HandlerResult::NORMAL_LISTEN;
  }

  HandlerResult HandleWrite() override {
    abort();
  }

  bool HasDataForWriting() override {
    return false;
  }

  
};
