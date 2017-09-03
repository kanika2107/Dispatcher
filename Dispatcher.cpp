#include "Dispatcher.h"
#include "MessageRecieveSocket.h"

#include <cassert>
#include <err.h>
#include <iostream>
#include <sys/select.h>
#include <vector>



void Dispatcher::register_socket(std::unique_ptr<Socket> socket) {
  int fd = socket->fd();
  assert(ptr_TimeHandlerClass->fds.count(fd) == 0);
  ptr_TimeHandlerClass->fds[fd] = std::move(socket);

}

void Dispatcher::register_policy(int fd,std::unique_ptr<TimerPolicy> timer_policy)
{
  std::cout << "Registering timer policy" << std::endl;
  ptr_TimeHandlerClass->timer_policies[fd] = std::move(timer_policy);
}

void Dispatcher::register_timer(int fd,std::unique_ptr<Timer> read_timer,std::unique_ptr<Timer> write_timer)
{
  std::cout << "Registering timer" << std::endl;
  
  if(ptr_TimeHandlerClass->timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
  {
    const int interval = ptr_TimeHandlerClass->timer_policies[fd]->interval;
    ptr_TimeHandlerClass->timers[fd].first = std::move(read_timer);
    ptr_TimeHandlerClass->timers[fd].second = std::move(write_timer);
    ptr_TimeHandlerClass->timers_queue.push(std::make_pair(time(0)+interval,std::make_pair(fd,'R')));
    ptr_TimeHandlerClass->timers_queue.push(std::make_pair(time(0)+interval,std::make_pair(fd,'W')));
  }
  
}


Socket* Dispatcher::getSocket_ptr(int fd)
{
  return ptr_TimeHandlerClass->fds[fd].get();
}



void Dispatcher::CallHandlers(int fd_bound,fd_set &readset, fd_set &writeset,int res)
{
  for (int fd = 0; fd < fd_bound; ++fd) {
      bool remove = false;
     
     //NOTE: HandreWrite is being called before HandleRead to avoid multiple reads being called in the previous approach and overwriting the write data
      if (FD_ISSET(fd, &writeset)) { 
        auto result = ptr_TimeHandlerClass->fds[fd]->HandleWrite();
        if (result == Socket::HandlerResult::REMOVE) remove = true;
        else if(result==Socket::HandlerResult::NORMAL) ptr_TimeHandlerClass->UpdateWriteTimer(fd);
        --res;
      }

       if (FD_ISSET(fd, &readset)) {
        auto result = ptr_TimeHandlerClass->fds[fd]->HandleRead();
        if(result== Socket::HandlerResult::NORMAL_LISTEN) //Add to listeners if this socket is a listen socket
          listeners.push_back(fd);
  
        else if(result==Socket::HandlerResult::NORMAL)
          ptr_TimeHandlerClass->UpdateReadTimer(fd);

        if(ptr_TimeHandlerClass->fds[fd]->HasDataForWriting()) FD_SET(fd, &writeset);
        
        if (result == Socket::HandlerResult::REMOVE) 
          {
            std::cout << "Going to close" << std::endl;
            remove = true;
          }
        --res;
      }

      if (remove)
      {
        std::cout << "Connection closed" << std::endl;
        ptr_TimeHandlerClass->CloseConnection(fd);
      }
      if (res == 0) break;
    }
}
void Dispatcher::run() {

  struct timeval tv;
  tv.tv_sec=0;
  tv.tv_usec=0;

  while (!ptr_TimeHandlerClass->fds.empty()) {

    // Create fd_sets
    fd_set readset;
    fd_set writeset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);

    // Populate fd_sets
    int max_fd{0};
    for (auto &pair : ptr_TimeHandlerClass->fds) {
      FD_SET(pair.first, &readset);
      if (pair.second->HasDataForWriting()) FD_SET(pair.first, &writeset);
      max_fd = std::max(max_fd, pair.first);
    }

    ptr_TimeHandlerClass->UpdatingTimerQueue(); //To remove the previous timeouts for which new orders came before their triggering
    bool timeout_occured = false;
    if(!ptr_TimeHandlerClass->timers_queue.empty())
    {

      int deadline = ptr_TimeHandlerClass->CheckTimeout();
      if(deadline<0)
      {
        timeout_occured=true;
        if(Socket::HandlerResult::REMOVE == ptr_TimeHandlerClass->ProcessTimeout())
          continue; 
        deadline=0;
      }
      tv.tv_sec = deadline;
      tv.tv_usec = 0;
    }

    // Call select
    int fd_bound = max_fd + 1;
    int res = select(fd_bound, &readset, &writeset, nullptr, &tv);
    if (res < 0) err(2, "select");

    // Process results
    CallHandlers(fd_bound,readset,writeset,res);

    /*if(timeout_occured==true)
    { //Process timeouts at the end after calling the handlers to not starve the connections
        ptr_TimeHandlerClass->ProcessTimeout(); 
    }*/
    
  }
}