#pragma once

#include "Socket.h"
#include "Timer.h"
#include "TimerPolicy.h"
#include "TimerHandler.h"


#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>


class Dispatcher {
  
  std::vector<int> listeners; //Vector of all current listeners 

public:
  
  std::unique_ptr<TimerHandler> ptr_TimeHandlerClass;  //This class manages all the timer related functionality and dispatcher has a pointer to this singleton class
  Dispatcher()
  {
    ptr_TimeHandlerClass = TimerHandler::getInstance();
  }
  void register_socket(std::unique_ptr<Socket> socket);
  void register_timer(int fd,std::unique_ptr<Timer> read_timer,std::unique_ptr<Timer> write_timer);
  void CloseConnection(int fd);
  void run();
  void CallHandlers(int fd_bound,fd_set &readset,fd_set &writeset,int res);
  void register_policy(int fd,std::unique_ptr<TimerPolicy> timer_policy);
  Socket* getSocket_ptr(int fd);
};
