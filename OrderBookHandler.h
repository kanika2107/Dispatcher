#pragma once

#include "MessageRecieveSocket.h"
#include "wire_messages.h"

#include <iostream>
#include <map>
#include <memory>


class MessageRecieveSocket;
//Singleton class for handling order book insertion, deletion and matching
class OrderBookHandler
{
private:
  
  static bool instanceFlag;
  static std::shared_ptr<OrderBookHandler> single;
  static std::map<int,std::pair<std::pair<int,uint32_t>,MessageRecieveSocket*>> order_book;

  //OrderBook 
  //order_book.first---->fd
  //order_book.second.first.first----->BUY or SELL
  //order_book.second.first.second-----> Order ID of this order
  //order_book.second.second-------->MessageRecieveSocket*
  
  OrderBookHandler()
  {

  }
public:
  static std::shared_ptr<OrderBookHandler> getInstance();
  void InsertOrderInBook(NewOrder* new_order,MessageRecieveSocket* ptr_socket,int fd);
  void Remove_from_OrderBook(int fd);
  int getCount(int fd);
  void match(bool& matched,int& matched_fd, int& to_match);
  MessageRecieveSocket* getMessageRecieveSocketFromBook(int matched_fd);
  uint32_t getOrderIDfromBook(int matched_fd);
  ~OrderBookHandler()
  {
    std::cout << "Singleton class is being destructed" << std::endl;
    instanceFlag=false;
  }
};