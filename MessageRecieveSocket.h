#pragma once

#include "OrderBookHandler.h"
#include "Socket.h"
#include "wire_messages.h"

#include <algorithm>
#include <cstring>
#include <err.h>
#include <iostream>
#include <map>
#include <sys/uio.h>
#include <unistd.h>



class MessageRecieveSocket : public Socket {
  std::shared_ptr<OrderBookHandler> ptr_OrderBookClass;
  std::string m_write_buffer;
  struct MessageHeader msgheader;
  struct Nack nackmessage;
  struct Ping pingmessage;
  struct Pong pongmessage;
  struct Exec execmessage;
  struct Ack ackmessage;
  struct iovec iov[2];
  //holds the mapping of the fd with client_token
  static std::map<int,uint64_t> users;

public:

  ~MessageRecieveSocket() = default;

  MessageRecieveSocket(int fd) {
    ptr_OrderBookClass = OrderBookHandler::getInstance();
    m_fd = fd;
  }

  HandlerResult PrepareHeartbeat()
  {
      std::cout << "Preparing Heartbeat from Timer callback" << std::endl;
      msgheader.type=MessageType::PING;
      msgheader.sequence_number = 1;
      msgheader.size=sizeof(Ping);

      srand (time(NULL));
      
      pingmessage.cookie=rand()%100;
      std::cout << pingmessage.cookie << std::endl;
      
      iov[0].iov_base = reinterpret_cast<char*>(&msgheader);
      iov[0].iov_len = sizeof(msgheader);
      iov[1].iov_base = reinterpret_cast<char*>(&pingmessage);
      iov[1].iov_len = sizeof(pingmessage);
      m_write_buffer="full"; //Mark that there is data to write
      std::cout << reinterpret_cast<MessageHeader*>(iov[0].iov_base)->size << std::endl;
      return HandlerResult::NORMAL;
  }

  HandlerResult CloseConnection()
  {

    return HandlerResult::REMOVE;
  }

  HandlerResult LoginHandler(char* buffer) 
  {
      if(users.count(m_fd)!=0) //Client was already logged in then simply return
        return HandlerResult::NORMAL;

      std::cout << "LoginHandler" << std::endl;
      Login* loginmessage = reinterpret_cast<Login*>(buffer);
      if(loginmessage->client_token==0 || loginmessage->client_token==1 || loginmessage->client_token==2) //Do validation on the UserID
      {
        users[m_fd] = loginmessage->client_token;
        //Prepare LOGIN_SUCCESS
        msgheader.type = MessageType::LOGIN_SUCCESS;
        msgheader.sequence_number = 1;
        msgheader.size = 0;
        iov[0].iov_base = reinterpret_cast<char*>(&msgheader);
        iov[0].iov_len = sizeof(msgheader);
        iov[1].iov_base = reinterpret_cast<char*>(&msgheader);
        iov[1].iov_len = 0;
        m_write_buffer="full"; //Mark that there is data to write
        return HandlerResult::NORMAL;
      }
      else
      {
        //Reject this user
        //Prepare LOGIN_FAILURE
        msgheader.type = MessageType::LOGIN_FAILURE;
        msgheader.sequence_number = 1;
        msgheader.size = 0;
        iov[0].iov_base = reinterpret_cast<char*>(&msgheader);
        iov[0].iov_len = sizeof(msgheader);
        m_write_buffer="full"; //Mark that there is data to write
        return HandlerResult::REMOVE;
      }
  }

  HandlerResult PingHandler(char* buffer)
  {

      //close the connection if client is not logged in and sends some message
      if(users.count(m_fd)==0)
        return HandlerResult::REMOVE;

      std::cout << "PingHandler" << std::endl;
      Ping* ping_message = reinterpret_cast<Ping*>(buffer);
      std::cout << ping_message->cookie << std::endl;
     
      //Prepare a Pong in response to the Ping

      msgheader.type=MessageType::PONG;
      msgheader.sequence_number = 1;
      msgheader.size=sizeof(Pong);

      pongmessage.cookie=ping_message->cookie;
      
      iov[0].iov_base = reinterpret_cast<char*>(&msgheader);
      iov[0].iov_len = sizeof(msgheader);
      iov[1].iov_base = reinterpret_cast<char*>(&pongmessage);
      iov[1].iov_len = sizeof(pongmessage);
      m_write_buffer="full"; //Mark that there is data to write
      return HandlerResult::NORMAL;
  }

  HandlerResult PrepareNackMsg(NewOrder* new_order)
  {
     //Send Nacks
      std::cout << "Nack Message is being sent" << std::endl;
      msgheader.type=MessageType::ORDER_NACK;
      msgheader.sequence_number = 2;
      msgheader.size=sizeof(Nack);
 
      nackmessage.order_id=new_order->order_id;
      
      iov[0].iov_base = reinterpret_cast<char*>(&msgheader);
      iov[0].iov_len = sizeof(msgheader);
      iov[1].iov_base = reinterpret_cast<char*>(&nackmessage);
      iov[1].iov_len = sizeof(nackmessage);
      m_write_buffer="full";
      return HandlerResult::NORMAL;
  }

  HandlerResult PrepareAckMsg(NewOrder* new_order)
  {
      //Send an Ack
      std::cout << "Ack Message is being sent" << std::endl;
      msgheader.type=MessageType::ORDER_ACK;
      msgheader.sequence_number = 2;
      msgheader.size=sizeof(Ack);
   
      ackmessage.order_id=new_order->order_id;
          
      iov[0].iov_base = reinterpret_cast<char*>(&msgheader);
      iov[0].iov_len = sizeof(msgheader);
      iov[1].iov_base = reinterpret_cast<char*>(&ackmessage);
      iov[1].iov_len = sizeof(ackmessage);
      m_write_buffer="full";
      return HandlerResult::NORMAL;
  }

  inline void PrepareExecMsg(MessageRecieveSocket* ptr_to_socket, uint32_t order_id, Side side) 
  {
      std::cout << "EXEC Message is being prepared" << std::endl;
      //send an EXEC for this order
      
      //struct MessageHeader msgheader;
      ptr_to_socket->msgheader.type=MessageType::EXEC;
      ptr_to_socket->msgheader.sequence_number = 2;
      ptr_to_socket->msgheader.size=sizeof(Exec);

      //struct Exec execmessage;
      ptr_to_socket->execmessage.order_id=order_id;
      ptr_to_socket->execmessage.qty=1;
      ptr_to_socket->execmessage.price=1;
      ptr_to_socket->execmessage.symbol=0;
      ptr_to_socket->execmessage.side=side;

      ptr_to_socket->iov[0].iov_base = reinterpret_cast<char*>(&(ptr_to_socket->msgheader));
      ptr_to_socket->iov[0].iov_len = sizeof(ptr_to_socket->msgheader);
      ptr_to_socket->iov[1].iov_base = reinterpret_cast<char*>(&(ptr_to_socket->execmessage));
      ptr_to_socket->iov[1].iov_len = sizeof(ptr_to_socket->execmessage);
      ptr_to_socket->m_write_buffer="full";

  }

  HandlerResult NewOrderHandler(char* buffer)
  {

    //close the connection if client is not logged in and sends some message
    if(users.count(m_fd)==0)
      return HandlerResult::REMOVE;

    std::cout << "NewOrderRecieved" << std::endl;
    NewOrder* new_order=reinterpret_cast<NewOrder*>(buffer);
    //Validation of the order
    
    if(new_order->price!=1 || new_order->qty!=1 || new_order->symbol!=0 || ptr_OrderBookClass->getCount(m_fd)!=0)
    {
      return PrepareNackMsg(new_order); 
    }
    else
    {
      //Try to match the order from the order book
      bool matched=false;
      int to_match=(new_order->side==Side::BUY ? 0 : 1);
      int matched_fd;
      
       //check in the order book if it matches
      ptr_OrderBookClass->match(matched,matched_fd,to_match);
     

      if(!matched)
      {
          ptr_OrderBookClass->InsertOrderInBook(new_order,this,m_fd);
          return PrepareAckMsg(new_order);   
      }
      else
      {
          std::cout << "MACTHED ORDER!!" << std::endl;
          Side side = (to_match==0 ? Side::SELL : Side::BUY);

          //Prepare EXEC for this order
          PrepareExecMsg(this,new_order->order_id,new_order->side);

          //Prepare EXEC for matched order
          PrepareExecMsg(ptr_OrderBookClass->getMessageRecieveSocketFromBook(matched_fd),ptr_OrderBookClass->getOrderIDfromBook(matched_fd),side);

          //Delete this entry from the order book since it has been matched
          ptr_OrderBookClass->Remove_from_OrderBook(matched_fd);
          return HandlerResult::NORMAL;

      }

    }

  }

  HandlerResult HandleRead() override {

    //Read the message
    ssize_t bytes_read;
    char msg_header[sizeof(MessageHeader)];
    struct iovec iov[1];
    int iovcnt;

    iov[0].iov_base = msg_header;
    iov[0].iov_len  = sizeof(msg_header);
    iovcnt = sizeof(iov)/sizeof(struct iovec);
    bytes_read = readv(m_fd,iov,iovcnt);

    //Read the header
    MessageHeader* msgheader = reinterpret_cast<MessageHeader*>(msg_header);

    char buffer[msgheader->size];  //Now read the message
    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);
    iovcnt = sizeof(iov)/sizeof(struct iovec);
    bytes_read = readv(m_fd,iov,iovcnt);

     if (bytes_read < 0) {
      if (errno != EAGAIN) warn("read");
      return HandlerResult::NORMAL;
    } else if (bytes_read == 0) {

      users.erase(m_fd);
      m_write_buffer.erase(0,m_write_buffer.length());
      ptr_OrderBookClass->Remove_from_OrderBook(m_fd);
      
      return HandlerResult::REMOVE; //Connection was closed.
    }
    
    if(msgheader->type==MessageType::LOGIN)
      return LoginHandler(buffer); 

    else if(msgheader->type==MessageType::PING)
      return PingHandler(buffer);

    else if(msgheader->type==MessageType::NEW_ORDER)
      return NewOrderHandler(buffer);
    
  }

  HandlerResult HandleWrite() override {
    ssize_t res;
    res = writev(m_fd,iov,2);
    std::cout << res << std::endl;
    std::cout << reinterpret_cast<MessageHeader*>(iov[0].iov_base)->size << std::endl;
    if (res < 0) {
      if (errno != EAGAIN) warn("writev");
      return HandlerResult::NORMAL;
    }
    m_write_buffer.erase(0,m_write_buffer.length());
    return HandlerResult::NORMAL;
  }

  bool HasDataForWriting() override {
    return !m_write_buffer.empty();
  }
};