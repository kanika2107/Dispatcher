#include "Dispatcher.h"
#include "MessageRecieveSocket.h"
#include "Session.h"

#include <cassert>
#include <err.h>
#include <iostream>
#include <sys/select.h>
#include <vector>



void Dispatcher::register_socket(std::unique_ptr<Socket> socket) {
  int fd = socket->fd();
  assert(ptr_TimeHandlerClass.fds.count(fd) == 0);
  ptr_TimeHandlerClass.fds[fd] = std::move(socket);

}

void Dispatcher::register_policy(int fd,std::unique_ptr<TimerPolicy> timer_policy)
{
  std::cout << "Registering timer policy" << std::endl;
  ptr_TimeHandlerClass.timer_policies[fd] = std::move(timer_policy);
}

void Dispatcher::register_timer(int fd,std::unique_ptr<Timer> read_timer,std::unique_ptr<Timer> write_timer)
{
  std::cout << "Registering timer" << std::endl;
  
  if(ptr_TimeHandlerClass.timer_policies[fd]->GetStatus() ==  TimerPolicy::TimerStatus::ENABLE_TIMER)
  {
    std::cout << read_timer->getTimerType() << std::endl;
    ptr_TimeHandlerClass.timers_set.insert(std::move(read_timer));
    ptr_TimeHandlerClass.timers_set.insert(std::move(write_timer));
  }
  
}


Socket* Dispatcher::getSocket_ptr(int fd)
{
  return ptr_TimeHandlerClass.fds[fd].get();
}



void Dispatcher::CallHandlers(int fd_bound,fd_set &readset, fd_set &writeset,int res)
{
  for (int fd = 0; fd < fd_bound; ++fd) {
      bool remove = false;
          
       if (FD_ISSET(fd, &readset)) {
        auto result = ptr_TimeHandlerClass.fds[fd]->HandleRead();

        if(result== Socket::HandlerResult::NORMAL_LISTEN) //Add to listeners if this socket is a listen socket
          listeners.push_back(fd);
  
        else if(result==Socket::HandlerResult::NORMAL)
          ptr_TimeHandlerClass.UpdateReadTimer(fd);

        if(ptr_TimeHandlerClass.fds[fd]->HasDataForWriting()) FD_SET(fd, &writeset);
        
        if (result == Socket::HandlerResult::REMOVE) 
          {
            std::cout << "Going to close" << std::endl;
            remove = true;
          }
        --res;
      }

      if (FD_ISSET(fd, &writeset)) { 
        auto result = ptr_TimeHandlerClass.fds[fd]->HandleWrite();
        if (result == Socket::HandlerResult::REMOVE) remove = true;
        else if(result==Socket::HandlerResult::NORMAL) ptr_TimeHandlerClass.UpdateWriteTimer(fd);
        --res;
      }

      if (remove)
      {
        std::cout << "Connection closed" << std::endl;
        ptr_TimeHandlerClass.CloseConnection(fd);
      }
      if (res == 0) break;
    }
}
void Dispatcher::run() {


  struct timeval tv;
  tv.tv_sec=0;
  tv.tv_usec=0;

  //Register matching timer
  ptr_TimeHandlerClass.timers_set.insert(std::unique_ptr<Timer> (new MatchingTimer(time(0)+0.2,ptr_TimeHandlerClass.order_book_helper,-1)));
  
  while (!ptr_TimeHandlerClass.fds.empty()) {

    // Create fd_sets
    fd_set readset;
    fd_set writeset;
    FD_ZERO(&readset);
    FD_ZERO(&writeset);

    
    ptr_TimeHandlerClass.UpdatingTimerQueue(); //To remove the previous timeouts for which new orders came before their triggering
    if(!ptr_TimeHandlerClass.timers_set.empty())
    {

      int deadline = ptr_TimeHandlerClass.CheckTimeout();
      if(deadline<0)
      {
        if(Socket::HandlerResult::REMOVE == ptr_TimeHandlerClass.ProcessTimeout()) //Callback for close connection was encountered
          continue; 
        deadline=0;
      }
      tv.tv_sec = deadline;
      tv.tv_usec = 0;
    }

    // Populate fd_sets
    int max_fd{0};
    for (auto &pair : ptr_TimeHandlerClass.fds) {
      FD_SET(pair.first, &readset);
      if (pair.second->HasDataForWriting()) FD_SET(pair.first, &writeset);
      max_fd = std::max(max_fd, pair.first);
    }

    // Call select
    int fd_bound = max_fd + 1;
    int res = select(fd_bound, &readset, &writeset, nullptr, &tv);
    if (res < 0) err(2, "select");

    // Process results
    CallHandlers(fd_bound,readset,writeset,res);

  }
}