#pragma once

#include "LineReverseSocket.h"
#include "MessageRecieveSocket.h"

#include <time.h> 
#include <unistd.h>


class Timer {
  
protected:
  Timer() = default;
  time_t next_timeout; //Absolute time for next timeout
  

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
  virtual ~Timer(){}
};

class MessageRecieveSocketTimer: public Timer
{
public:
  MessageRecieveSocketTimer(time_t timeout,Socket* socket)
  {
    next_timeout=timeout;
    TimerCallback=std::bind(&MessageRecieveSocket::PrepareHeartbeat,dynamic_cast<MessageRecieveSocket*>(socket));
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
  LineReverseSocketTimer(time_t timeout,Socket* socket)
  {
    next_timeout=timeout;
    TimerCallback=std::bind(&LineReverseSocket::PrepareHeartbeat,dynamic_cast<LineReverseSocket*>(socket));
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