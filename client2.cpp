#include "wire_messages.h"

#include <arpa/inet.h>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/uio.h>

static constexpr size_t BUFFER_SIZE = 2048;
int listen_socket(uint16_t portno) {

  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) err(2, "Creating socket");
  sockaddr_in addr;
  memset(&addr, '0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portno);
  int status=inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
  if(status<=0) err(2,"pton error");

  int res = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    std::cout << fd << std::endl;

  if(res<0) err(2,"Connect failed");


  /////////////////////////////////////////////LOGIN MESSAGE///////////////////////(Testing for successsful login)
  struct MessageHeader msgheader1;
  msgheader1.type=MessageType::LOGIN;
  msgheader1.sequence_number = 1;
  msgheader1.size=sizeof(Login);

  struct Login loginmessage;
  loginmessage.client_token=0;
  loginmessage.last_seqnum=-1;

  char* msg_header1 = reinterpret_cast<char*>(&msgheader1);
  char* login_message = reinterpret_cast<char*>(&loginmessage);
  struct iovec iov1[2];
  ssize_t nwritten;
  iov1[0].iov_base = msg_header1;
  iov1[0].iov_len = sizeof(msgheader1);
  iov1[1].iov_base = login_message;
  iov1[1].iov_len = sizeof(loginmessage);
  nwritten = writev(fd,iov1,2);
  //std::cout << nwritten << std::endl; 
  if(nwritten<0) err(2,"writev failed");


////////////////////////////////////////////////////////////////////////////////

    ssize_t bytes_read;
    char msg_header2[sizeof(MessageHeader)];
   
    int iovcnt2;
    struct iovec iov2[1];
    iov2[0].iov_base = msg_header2;
    iov2[0].iov_len  = sizeof(msg_header2);
    iovcnt2 = sizeof(iov2)/sizeof(struct iovec);
    bytes_read = readv(fd,iov2,iovcnt2);
    MessageHeader* msgheader2 = reinterpret_cast<MessageHeader*>(msg_header2);
    char dump[msgheader2->size];
    iov2[0].iov_base = dump;
    iov2[0].iov_len  = sizeof(dump);
    iovcnt2 = sizeof(iov2)/sizeof(struct iovec);
    bytes_read = readv(fd,iov2,iovcnt2);
    //std::cout << bytes_read << std::endl;
    if(bytes_read<0) err(2,"readv failed");
    //Read the header   
    if(msgheader2->type==MessageType::LOGIN_SUCCESS)
    {
      std::cout << "Recieved Login Successful from server" << std::endl;
    }
    else if(msgheader2->type==MessageType::LOGIN_FAILURE)
    {
      std::cout << "Recieved Login failure from server" << std::endl;
    }


    //sleep(3);
    /////////////////////////RECIEVE A PING//////////////////////////////////////


   /* struct iovec iov8[1];
    char msg_header8[sizeof(MessageHeader)];
    iov8[0].iov_base = msg_header8;
    iov8[0].iov_len  = sizeof(msg_header8);
    int iovcnt8 = sizeof(iov8)/sizeof(struct iovec);
    bytes_read = readv(fd,iov8,iovcnt8);
    MessageHeader* msgheader8 = reinterpret_cast<MessageHeader*>(msg_header8);
    char buffer8[msgheader8->size];
    iov8[0].iov_base = buffer8;
    iov8[0].iov_len = sizeof(buffer8);
    iovcnt8 = sizeof(iov8)/sizeof(struct iovec);
    bytes_read = readv(fd,iov8,iovcnt8);
    if(bytes_read<0) err(2,"readv failed");
    //Read the header
    if(msgheader8->type==MessageType::PING)
    {
      Ping* pingmessage = reinterpret_cast<Ping*>(buffer8);
      std::cout << pingmessage->cookie << std::endl;
    }*/

    //sleep(4);
/////////////////////////////////////////////////PING//////////////////////////////////////
  
    struct iovec iov3[2];
   
  srand (time(NULL));
  struct MessageHeader msgheader3;
  msgheader3.type=MessageType::PING;
  msgheader3.sequence_number = 2;
  msgheader3.size=sizeof(Ping);

  struct Ping pingmessage;
  pingmessage.cookie=rand()%100;
  std::cout << pingmessage.cookie << std::endl;

  iov3[0].iov_base = reinterpret_cast<char*>(&msgheader3);
  iov3[0].iov_len = sizeof(msgheader3);
  iov3[1].iov_base = reinterpret_cast<char*>(&pingmessage);
  iov3[1].iov_len = sizeof(pingmessage);
  nwritten = writev(fd,iov3,2);
  //std::cout << nwritten << std::endl; 
  if(nwritten<0) err(2,"writev failed");
 
   //sleep(1);

//////////////////////////////////////////////RECIEVE A PONG/////////////////////////////////////////////
    struct iovec iov4[1];
    char msg_header4[sizeof(MessageHeader)];
    
    iov4[0].iov_base = msg_header4;
    iov4[0].iov_len  = sizeof(msg_header4);
    int iovcnt4 = sizeof(iov4)/sizeof(struct iovec);
    bytes_read = readv(fd,iov4,iovcnt4);
    MessageHeader* msgheader4 = reinterpret_cast<MessageHeader*>(msg_header4);
    char buffer[msgheader4->size];
    iov4[0].iov_base = buffer;
    iov4[0].iov_len = sizeof(buffer);
    iovcnt4 = sizeof(iov4)/sizeof(struct iovec);
    bytes_read = readv(fd,iov4,iovcnt4);
    std::cout << bytes_read << std::endl;
    if(bytes_read<0) err(2,"readv failed");
    //Read the header
   
    std::cout << msgheader4->size << std::endl;
    if(msgheader4->type==MessageType::PONG)
    {
      Pong* pongmessage = reinterpret_cast<Pong*>(buffer);
      std::cout << pongmessage->cookie << std::endl;
    }

    // sleep(1);
    ////////////////////////////////SEND A NEW ORDER/////////////////////////////////////////////
 
  struct iovec iov5[2];
  struct MessageHeader msgheader5;
  msgheader5.type=MessageType::NEW_ORDER;
  msgheader5.sequence_number = 3;
  msgheader5.size=sizeof(NewOrder);

  struct NewOrder neworder;
  neworder.price=1;
  neworder.qty=1;
  neworder.symbol=0;
  neworder.order_id=12;
  neworder.side=Side::SELL;


  char* msg_header5 = reinterpret_cast<char*>(&msgheader5);
  char* new_order = reinterpret_cast<char*>(&neworder);
  iov5[0].iov_base = msg_header5;
  iov5[0].iov_len = sizeof(msgheader5);
  iov5[1].iov_base = new_order;
  iov5[1].iov_len = sizeof(neworder);
  nwritten = writev(fd,iov5,2);
  std::cout << nwritten << std::endl; 
  if(nwritten<0) err(2,"writev failed");
  //sleep(1);

  ////////////////////////////////RECIEVE ACK OR NACK OR EXEC FOR THIS ORDER////////////////////////////////////////////////////
    struct iovec iov6[1];
    char msg_header6[sizeof(MessageHeader)];
   
    iov6[0].iov_base = msg_header6;
    iov6[0].iov_len  = sizeof(msg_header6);
    int iovcnt6 = sizeof(iov6)/sizeof(struct iovec);
    bytes_read = readv(fd,iov6,iovcnt6);
    MessageHeader* msgheader6 = reinterpret_cast<MessageHeader*>(msg_header6);
    char buffer6[msgheader6->size];
    iov6[0].iov_base = buffer6;
    iov6[0].iov_len = sizeof(buffer6);
    iovcnt6 = sizeof(iov6)/sizeof(struct iovec);
    bytes_read = readv(fd,iov6,iovcnt6);
    if(bytes_read<0) err(2,"readv failed");
    //Read the header
    if(msgheader6->type==MessageType::ORDER_NACK)
    {
      Nack* nackmessage = reinterpret_cast<Nack*>(buffer6);
      //std::cout << nackmessage->order_id << std::endl;
    }

     else if(msgheader6->type==MessageType::ORDER_ACK)
    {
      Ack* ackmessage = reinterpret_cast<Ack*>(buffer6);
      //std::cout << ackmessage->order_id << std::endl;
    }

    else if(msgheader6->type==MessageType::EXEC)
    {
      Exec* execmessage = reinterpret_cast<Exec*>(buffer6);
      std::cout << execmessage->order_id << std::endl;
    }
    sleep(1);
    

  }


int main(int argc, char **argv) {
  std::vector<std::string> args{argv, argv + argc};
  if (argc != 2) errx(1, "Need a port number");
  int listen_fd = listen_socket(uint16_t(std::stoi(args[1])));
  close(listen_fd);
  return 0;
}