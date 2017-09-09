#pragma once

#include "LineReverseSocket.h"
#include "MessageRecieveSocket.h"
#include "OrderBookHelper.h"

#include <time.h> 
#include <unistd.h>


class Timer {
  
protected:
  Timer() = default;
  time_t next_timeout; //Absolute time for next timeout
  int m_fd;
  char m_read_or_write;
  

public:

  Timer(const Timer &) = delete;
  
  //Callback object
  std::function<Socket::HandlerResult()> TimerCallback;
  
  virtual void changeCallbacktoPing(Socket* socket)=0;

  virtual void changeCallbacktoClose(Socket* socket)=0;

  time_t getTimeout()
  {
    return next_timeout;
  }

  void setTimeout(time_t timeout)
  {
    next_timeout=timeout;
  }

  int fd()
  {
    return m_fd;
  }

  char getTimerType()
  {
    return m_read_or_write;
  }


  virtual ~Timer(){}
};

class MessageRecieveSocketTimer: public Timer
{
public:
  MessageRecieveSocketTimer(time_t timeout,Socket* socket,int fd,char read_or_write)
  {
    next_timeout=timeout;
    TimerCallback=std::bind(&MessageRecieveSocket::PrepareHeartbeat,dynamic_cast<MessageRecieveSocket*>(socket));
    m_fd=fd;
    m_read_or_write=read_or_write;
    //read_or_write=read_or_write;
  }
  void changeCallbacktoPing(Socket* socket) override
  {
    TimerCallback=std::bind(&MessageRecieveSocket::PrepareHeartbeat,dynamic_cast<MessageRecieveSocket*>(socket));
  }
  void changeCallbacktoClose(Socket* socket) override
  {
    TimerCallback=std::bind(&MessageRecieveSocket::CloseConnection,dynamic_cast<MessageRecieveSocket*>(socket));
  }

};

//Note: This adds for flexibility in design. We can have multiple types of connection sockets that do different things but are handled by one dispatcher
class LineReverseSocketTimer: public Timer
{
public:
  LineReverseSocketTimer(time_t timeout,Socket* socket,int fd,char read_or_write)
  {
    next_timeout=timeout;
    TimerCallback=std::bind(&LineReverseSocket::PrepareHeartbeat,dynamic_cast<LineReverseSocket*>(socket));
    m_fd=fd;
    read_or_write=read_or_write;
  }
  void changeCallbacktoPing(Socket* socket) override
  {
    TimerCallback=std::bind(&LineReverseSocket::PrepareHeartbeat,dynamic_cast<LineReverseSocket*>(socket));
  }
  void changeCallbacktoClose(Socket* socket) override
  {
    TimerCallback=std::bind(&LineReverseSocket::CloseConnection,dynamic_cast<LineReverseSocket*>(socket));
  }

};


class MatchingTimer: public Timer
{

  
public:
  std::function<void()> TimerCallback;
  MatchingTimer(time_t timeout,OrderBookHelper& helper,int fd)
  {
    next_timeout=timeout;
    TimerCallback=std::bind(&OrderBookHelper::match_orders,helper);
    m_fd=fd;
  }
  void changeCallbacktoPing(Socket* socket) override
  {
    abort();
  }
  void changeCallbacktoClose(Socket* socket) override
  {
    abort();
  }
};