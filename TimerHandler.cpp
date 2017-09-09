#include "Socket.h"
#include "TimerHandler.h"



#include <iostream>

void TimerHandler::UpdatingTimerQueue()
{
    //std::cout << "Updating Timer Queue" << std::endl;
    while(!timers_set.empty())
    {
      Timer* top_elem = (*timers_set.begin()).get();
      int match_fd = top_elem->fd();
      if(match_fd==-1)
      {
        return;
      }
      if(fds.count(match_fd)!=0) //This is zero if the client disconnects.So we need to pop the corresponding  timeouts related to this fd
      {
        if(top_elem->getTimerType()=='R')
        {
          if(top_elem->getTimeout()==fds[match_fd]->getReadTimeout())
          {
            break;
          }
        }
        else if(top_elem->getTimerType()=='W')
        {
          if(top_elem->getTimeout()==fds[match_fd]->getWriteTimeout())
            break;
        }
      } 
      timers_set.erase(timers_set.begin());
    }
}

//dynamic_cast<TimerObject*>(timers_set.Top().get())!= nullptr

void TimerHandler::UpdateAfterReadTimeout(int fd)
{
    if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
    {
      const int interval = timer_policies[fd]->interval;
      fds[fd]->setReadTimeout(time(0)+interval);
      if(dynamic_cast<MessageRecieveSocket*>(fds[fd].get())!=nullptr)
      {
        std::unique_ptr<Timer> temp(new MessageRecieveSocketTimer(time(0)+interval,fds[fd].get(),fd,'R'));
        temp->changeCallbacktoClose(fds[fd].get());
        timers_set.insert(std::move(temp));
      }
    }

}

void TimerHandler::UpdateAfterWriteTimeout(int fd)
{
      if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
      {
        const int interval = timer_policies[fd]->interval;
        fds[fd]->setWriteTimeout(time(0)+interval);
        if(dynamic_cast<MessageRecieveSocket*>(fds[fd].get())!=nullptr)
        {
          std::unique_ptr<Timer> temp(new MessageRecieveSocketTimer(time(0)+interval,fds[fd].get(),fd,'W'));
          timers_set.insert(std::move(temp));
        }
      }
}

void TimerHandler::UpdateWriteTimer(int fd)
{
      if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER){
        const int interval = timer_policies[fd]->interval;
        std::cout << "Updating the write timer" << std::endl;
        //Update the write timer in the map
        fds[fd]->setWriteTimeout(time(0)+interval);
        //Push the new timeout to the queue
        if(dynamic_cast<MessageRecieveSocket*>(fds[fd].get())!=nullptr)
        {
          std::unique_ptr<Timer> temp(new MessageRecieveSocketTimer(time(0)+interval,fds[fd].get(),fd,'W'));
          timers_set.insert(std::move(temp));
        }
      }
}

void TimerHandler::UpdateReadTimer(int fd)
{
      
      if(timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
      {
        const int interval = timer_policies[fd]->interval;
        std::cout << "Updating the read timer" << std::endl;
        //Update the read timer in the map
        fds[fd]->setReadTimeout(time(0)+interval);
        //Push the new timeout to the queue
        if(dynamic_cast<MessageRecieveSocket*>(fds[fd].get())!=nullptr)
        {

          std::unique_ptr<Timer> temp(new MessageRecieveSocketTimer(time(0)+interval,fds[fd].get(),fd,'R'));
          timers_set.insert(std::move(temp));

        }
      }
}

int TimerHandler::CheckTimeout()
{
      return (*timers_set.begin()).get()->getTimeout() - time(0);;
} 

void TimerHandler::CloseConnection(int fd)
{

  fds.erase(fd);
  timer_policies.erase(fd);
  std::cout << "Closing client connection" << std::endl;
 
}

Socket::HandlerResult TimerHandler::ProcessTimeout()
{
      std::cout << "Timeout Occurred" << std::endl;
      Socket::HandlerResult result=Socket::HandlerResult::NORMAL;
      //Timeout occurred
      int fd=(*timers_set.begin()).get()->fd();
      if(fd==-1)
      {
        std::cout << "Reached here " << std::endl;
        dynamic_cast<MatchingTimer*>((*timers_set.begin()).get())->TimerCallback();
        timers_set.erase(timers_set.begin());
        timers_set.insert(std::unique_ptr<Timer> (new MatchingTimer(time(0)+0.2,order_book_helper,-1)));
        return result;
      }
      
      if((*timers_set.begin()).get()->getTimerType()=='R')
      {
          result = (*timers_set.begin()).get()->TimerCallback();
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
          result = (*timers_set.begin()).get()->TimerCallback();
          UpdateAfterWriteTimeout(fd);
      }
}