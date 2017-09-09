#include "Dispatcher.h"
#include "LineReverseSocket.h"
#include "ListenSocket.h"
#include "MessageRecieveSocket.h"
#include "Session.h"
#include "Socket.h"
#include "Timer.h"
#include "TimerPolicy.h"


#include <err.h>
#include <map>
#include <stdlib.h>


int main(int argc, char **argv) {
  if (argc < 3) errx(1, "Need a port number and number of sessions");
  uint16_t port = atoi(argv[1]);
   
  static std::map<uint64_t,Session*> sessions;
  int no_sessions = atoi(argv[2]);
  for(int i=0;i<no_sessions;i++)
  {
  	sessions[i] = new Session();
  }


 
  static OrderBook order_book;
  static OrderBookHelper order_book_helper(sessions,order_book);
  TimerHandler timer_handler(order_book_helper);
  Dispatcher dispatch(timer_handler);

  dispatch.register_socket(std::unique_ptr<ListenSocket<MessageRecieveSocket,MessageRecieveSocketTimer,EnableTimerPolicy>> (new ListenSocket<MessageRecieveSocket,MessageRecieveSocketTimer,EnableTimerPolicy>(dispatch,port,sessions,order_book_helper) ));
  //dispatch.register_socket(std::unique_ptr<ListenSocket<LineReverseSocket,LineReverseSocketTimer,EnableTimerPolicy >>(new ListenSocket<LineReverseSocket,LineReverseSocketTimer,EnableTimerPolicy>(dispatch,4000,sessions)));
  dispatch.run();
  // NOT REACHED
}
