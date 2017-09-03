#pragma once

#include "Socket.h"
#include "Timer.h"
#include "TimerPolicy.h"

#include <queue>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

struct CompareByTime {
    constexpr bool operator()(std::pair<time_t,std::pair<int,char> > const &a,std::pair<time_t,std::pair<int,char> > const &b) const noexcept
    { 
      return a.first > b.first; 
    }
};


//Singleton class for handling timers, priority queue of timers nad processing the timeouts when they occur.
class TimerHandler
{
private:
  
  static bool instanceFlag;
  static std::unique_ptr<TimerHandler> single;
  
  TimerHandler()
  {

  }
public:

  std::map<int, std::unique_ptr<Socket>> fds;

  //Maps the fd to the policies they use
  std::map<int,std::unique_ptr<TimerPolicy> > timer_policies;

  //Map of Timers 
  //timers.first ----> fd
  //timers.second.first-------> Timer object for writing to this fd
  //timers.second.second------> Timer object for reading from this fd
  std::map<int,std::pair <std::unique_ptr<Timer> ,std::unique_ptr<Timer>  > > timers;
  
  //Priority Queue of Timers
  //timers_queue.fiirst-------->Absolute Time
  //timers_queue.second.first-->fd for this Time
  //timers_queue.second.second->Whether this time is for Reading 'R' or writing 'W'
  std::priority_queue<std::pair<time_t,std::pair<int,char> >,std::vector<std::pair<time_t,std::pair<int,char> > >, CompareByTime > timers_queue;

  static std::unique_ptr<TimerHandler> getInstance();
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
    instanceFlag=false;
  }
};