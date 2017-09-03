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


  /////////////////////////////////////////////LOGIN MESSAGE///////////////////////(testing for failure login)
  struct MessageHeader msgheader;
  msgheader.type=MessageType::LOGIN;
  msgheader.sequence_number = 1;
  msgheader.size=sizeof(Login);

  struct Login loginmessage;
  loginmessage.client_token=10;
  loginmessage.last_seqnum=-1;

  char* msg_header = reinterpret_cast<char*>(&msgheader);
  char* login_message = reinterpret_cast<char*>(&loginmessage);
  struct iovec iov[2];
  ssize_t nwritten;
  iov[0].iov_base = msg_header;
  iov[0].iov_len = sizeof(msgheader);
  iov[1].iov_base = login_message;
  iov[1].iov_len = sizeof(loginmessage);
  nwritten = writev(fd,iov,2);
  //std::cout << nwritten << std::endl; 
  if(nwritten<0) err(2,"writev failed");

  sleep(2);

    ssize_t bytes_read;
    char msg_header3[sizeof(MessageHeader)];
    int iovcnt3;
    struct iovec iov3[1];
    iov3[0].iov_base = msg_header3;
    iov3[0].iov_len  = sizeof(msg_header3);
    iovcnt3 = sizeof(iov3)/sizeof(struct iovec);
    bytes_read = readv(fd,iov3,iovcnt3);
    std::cout << bytes_read << std::endl;
    if(bytes_read<0) err(2,"readv failed");
    //Read the header
    MessageHeader* msgheader3 = reinterpret_cast<MessageHeader*>(msg_header3);
    if(msgheader3->type==MessageType::LOGIN_SUCCESS)
    {
      std::cout << "Recieved Login Successful from server" << std::endl;
    }
    else if(msgheader3->type==MessageType::LOGIN_FAILURE)
    {
      std::cout << "Recieved Login failure from server" << std::endl;
    }


  sleep(15);
}

int main(int argc, char **argv) {
  std::vector<std::string> args{argv, argv + argc};
  if (argc != 2) errx(1, "Need a port number");
  int listen_fd = listen_socket(uint16_t(std::stoi(args[1])));
  close(listen_fd);
  return 0;
}