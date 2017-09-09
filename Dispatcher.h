#pragma once

#include "Socket.h"
#include "Timer.h"
#include "TimerPolicy.h"
#include "TimerHandler.h"
#include "OrderBookHelper.h"


#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>


class Dispatcher {
  
  std::vector<int> listeners; //Vector of all current listeners 


public:
  
  TimerHandler& ptr_TimeHandlerClass;  //This class manages all the timer related functionality and dispatcher has a pointer to this singleton class
  Dispatcher(TimerHandler& timer_handler): ptr_TimeHandlerClass(timer_handler)
  {

  }
  void register_socket(std::unique_ptr<Socket> socket);
  void register_timer(int fd,std::unique_ptr<Timer> read_timer,std::unique_ptr<Timer> write_timer);
  void run();
  void CallHandlers(int fd_bound,fd_set &readset,fd_set &writeset,int res);
  void register_policy(int fd,std::unique_ptr<TimerPolicy> timer_policy);
  Socket* getSocket_ptr(int fd);
};
