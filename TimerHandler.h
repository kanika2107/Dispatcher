#pragma once

#include "Socket.h"
#include "Timer.h"
#include "TimerPolicy.h"

#include <queue>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

struct CompareByTime {
    bool operator()(std::unique_ptr<Timer> const &a,std::unique_ptr<Timer> const &b) const noexcept
    { 
      return a->getTimeout() > b->getTimeout(); 
    }
};


//Singleton class for handling timers, priority queue of timers nad processing the timeouts when they occur.
class TimerHandler
{
private:
  
  
public:
  OrderBookHelper& order_book_helper;
  TimerHandler(OrderBookHelper& orderBookhelper): order_book_helper(orderBookhelper)
  {

  }

  std::map<int, std::unique_ptr<Socket>> fds;

  //Maps the fd to the policies they use
  std::map<int,std::unique_ptr<TimerPolicy> > timer_policies;

  //Priority Queue of Timers
  std::set<std::unique_ptr<Timer>, CompareByTime > timers_set;

  void UpdatingTimerQueue();
  void UpdateAfterReadTimeout(int fd);
  void UpdateAfterWriteTimeout(int fd);
  void UpdateWriteTimer(int fd);
  void UpdateReadTimer(int fd);
  int CheckTimeout();
  Socket::HandlerResult ProcessTimeout();
  void CloseConnection(int fd);
  ~TimerHandler()
  {

  }
};