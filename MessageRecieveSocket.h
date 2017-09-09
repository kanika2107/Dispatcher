#pragma once

#include "OrderBookHelper.h"
#include "Socket.h"
#include "Order.h"
#include "wire_messages.h"

#include <algorithm>
#include <cstring>
#include <err.h>
#include <iostream>
#include <map>
#include <sys/uio.h>
#include <unistd.h>


static int constexpr BUFFER_SIZE = 1000000;

class Session;
class MessageRecieveSocket : public Socket {
  
  OrderBookHelper& ptr_OrderBookHelper;
  
  uint64_t client_token;
  struct MessageHeader msgheader;
  struct Nack nackmessage;
  struct Ping pingmessage;
  struct Pong pongmessage;
  struct Exec execmessage;
  struct Ack ackmessage;
  char temp_buffer[BUFFER_SIZE]; //temporary buffer used for writing
  //holds the mapping of the client_token with fd
  static std::map<uint64_t,int> users;
  
  int no_bytes_read; //denotes how many bytes were read
  bool was_header_read; //TRUE-> Header was read completely FALSE->Header was not read completely
  char m_msg_header[sizeof(MessageHeader)]; 
  char* msg_header;
  char m_read_buffer[BUFFER_SIZE];
  char* read_buffer;
  bool was_message_read; //TRUE-> Message was read completely FALSE->Message was not read completely
  MessageHeader* read_header;
  std::map<uint64_t,Session*>& m_sessions;

public:

  std::string m_write_buffer;
  bool flag_has_exec_data;
  ~MessageRecieveSocket() = default;

  MessageRecieveSocket(int fd,std::map<uint64_t,Session*>& sessions,time_t read_timeout, time_t write_timeout,OrderBookHelper& order_book_helper);

  HandlerResult CheckForError(int bytes_read);

  void PrepareMsgHeader(const MessageType& type,uint64_t seqnum,uint32_t size);



  HandlerResult PartialReadMessageHeader();
  
  HandlerResult PartialReadMessageContents();


   HandlerResult HandleRead() override;
   
    
  HandlerResult LogoutHandler();
 
  HandlerResult PrepareHeartbeat();
  

  HandlerResult CloseConnection();
  

  void AppendToWriteBuffer(const void* src,size_t size);
 
  HandlerResult LoginHandler(char* buffer); 
  

  HandlerResult PingHandler(char* buffer);
  

  HandlerResult PrepareNackMsg(NewOrder* new_order);
  
  

  HandlerResult PrepareAckMsg(NewOrder* new_order);
  

  HandlerResult NewOrderHandler(char* buffer);
 

  HandlerResult HandleWrite() override;
  
  bool HasDataForWriting() override; 
 
};