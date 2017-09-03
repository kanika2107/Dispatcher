#include "Dispatcher.h"
#include "LineReverseSocket.h"
#include "ListenSocket.h"
#include "MessageRecieveSocket.h"
#include "Socket.h"
#include "Timer.h"
#include "TimerPolicy.h"

#include <err.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) errx(1, "Need a port number");
  uint16_t port = atoi(argv[1]);
  Dispatcher dispatch;
  
  dispatch.register_socket(std::unique_ptr<ListenSocket<MessageRecieveSocket,MessageRecieveSocketTimer,EnableTimerPolicy>> (new ListenSocket<MessageRecieveSocket,MessageRecieveSocketTimer,EnableTimerPolicy>(dispatch,port) ));
  dispatch.register_socket(std::unique_ptr<ListenSocket<LineReverseSocket,LineReverseSocketTimer,EnableTimerPolicy >>(new ListenSocket<LineReverseSocket,LineReverseSocketTimer,EnableTimerPolicy>(dispatch,4000)));
  dispatch.run();
  // NOT REACHED
}
