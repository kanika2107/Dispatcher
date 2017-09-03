#include "Socket.h"
#include "TimerHandler.h"



#include <iostream>


bool TimerHandler::instanceFlag = false;
std::unique_ptr<TimerHandler> TimerHandler::single = NULL;

std::unique_ptr<TimerHandler> TimerHandler::getInstance()
{
  if(!instanceFlag)
  {
    std::unique_ptr<TimerHandler> resource(new TimerHandler());
    single = std::move(resource);
    instanceFlag = true;
    return std::move(single);
  }
  else
  {
    return std::move(single);
  }
}

void TimerHandler::UpdatingTimerQueue()
{
    //std::cout << "Updating Timer Queue" << std::endl;
    while(!timers_queue.empty())
    {
      std::pair<time_t,std::pair<int,char> > top_elem = timers_queue.top();
      int match_fd = top_elem.second.first;
      if(timers.count(match_fd)!=0) //This is zero if the client disconnects.So we need to pop the corresponding  timeouts related to this fd
      {
        if(top_elem.second.second=='R')
        {
          if(top_elem.first==timers[match_fd].first->getTimeout())
            break;
        }
        else if(top_elem.second.second=='W')
        {
          if(top_elem.first==timers[match_fd].second->getTimeout())
            break;
        }
      } 
      timers_queue.pop();
    }
}

void TimerHandler::UpdateAfterReadTimeout(int fd)
{
    if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
    {
      const int interval = timer_policies[fd]->interval;
      timers[fd].first->changeCallbacktoClose(fds[fd].get());
      timers[fd].first->setTimeout(time(0)+interval);
      timers_queue.push(std::make_pair(time(0)+interval,std::make_pair(fd,'R')));
    }

}

void TimerHandler::UpdateAfterWriteTimeout(int fd)
{
      if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
      {
        const int interval = timer_policies[fd]->interval;
        timers[fd].second->setTimeout(time(0)+interval);
        timers_queue.push(std::make_pair(time(0)+interval,std::make_pair(fd,'W')));
      }
}

void TimerHandler::UpdateWriteTimer(int fd)
{
      if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER){
        const int interval = timer_policies[fd]->interval;
        std::cout << "Updating the write timer" << std::endl;
        //Update the write timer in the map
        timers[fd].second->setTimeout(time(0)+interval);
        //Push the new timeout to the queue
        timers_queue.push(std::make_pair(time(0)+interval,std::make_pair(fd,'W')));
      }
}

void TimerHandler::UpdateReadTimer(int fd)
{
      
      if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
      {
        const int interval = timer_policies[fd]->interval;
        std::cout << "Updating the read timer" << std::endl;
        //Update the read timer in the map
        timers[fd].first->setTimeout(time(0)+interval);
        //Change the callback to send ping 
        timers[fd].first->changeCallbacktoPing(fds[fd].get());
        //Push the new timeout to the queue
        timers_queue.push(std::make_pair(time(0)+interval,std::make_pair(fd,'R')));
      }
}

int TimerHandler::CheckTimeout()
{
      return timers_queue.top().first - time(0);;
} 

void TimerHandler::CloseConnection(int fd)
{
  fds.erase(fd);
  timers.erase(fd);
  timer_policies.erase(fd);
}
Socket::HandlerResult TimerHandler::ProcessTimeout()
{
      std::cout << "Timeout Occurred" << std::endl;
      //Timeout occurred
      int fd=timers_queue.top().second.first;
      Socket::HandlerResult result;
      if(timers_queue.top().second.second=='R')
      {
          result = timers[fd].first->TimerCallback();
          if(result==Socket::HandlerResult::REMOVE) //No Pong recieved even after sending a Ping to the client on timeout
          {
              std::cout << "Closing Connection since no heartbeat was recieved" << std::endl;
              CloseConnection(fd);
              return Socket::HandlerResult::REMOVE;
          }
          else
          {
              UpdateAfterReadTimeout(fd);
          }

      }
      else
      {
          result = timers[fd].second->TimerCallback();
          UpdateAfterWriteTimeout(fd);
      }
}