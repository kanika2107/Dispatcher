#include "MessageRecieveSocket.h"
#include "OrderBookHandler.h"


#include <iostream>
#include <map>
#include <memory>

std::map<int,std::pair<std::pair<int,uint32_t>,MessageRecieveSocket*>>  OrderBookHandler::order_book;

bool OrderBookHandler::instanceFlag = false;
std::shared_ptr<OrderBookHandler> OrderBookHandler::single = NULL;

std::shared_ptr<OrderBookHandler> OrderBookHandler::getInstance()
{
  if(!instanceFlag)
  {
    std::cout << "Singleton class instantiation" << std::endl;
    std::shared_ptr<OrderBookHandler> resource(new OrderBookHandler());
    single = std::move(resource);
    instanceFlag = true;
    return single;
  }
  else
  {
    return single;
  }
}

void OrderBookHandler::InsertOrderInBook(NewOrder* new_order,MessageRecieveSocket* ptr_socket,int fd)
{
      std::cout << "Inserting in the order book" << std::endl;
      //Insert this order in the order book
      order_book[fd].first.first=(new_order->side==Side::BUY ? 0 : 1);
      order_book[fd].first.second=new_order->order_id;
      order_book[fd].second=ptr_socket;
}

void OrderBookHandler::Remove_from_OrderBook(int fd)
{
  //The connection is going to close ie a user is going to disconnect. Cancel or remove this user's order from the order book
    std::cout << "Removing your order if any from the order book since you are going to disconnect" << std::endl;
    if(order_book.count(fd)!=0)
    order_book.erase(fd);
}

void OrderBookHandler::match(bool& matched,int& matched_fd, int& to_match)
{
  std::cout << "Reference Count till now" << " " << single.use_count() << std::endl;
    for(auto &pair: order_book)
      {
        if(pair.second.first.first!=to_match) //Buy matched with a SELL
        {
          matched=true;
          matched_fd=pair.first;
          break;
        }
      }
}
int OrderBookHandler::getCount(int fd)
{
  return order_book.count(fd);
}

MessageRecieveSocket* OrderBookHandler::getMessageRecieveSocketFromBook(int matched_fd)
{
  return order_book[matched_fd].second;
}
  
uint32_t OrderBookHandler::getOrderIDfromBook(int matched_fd)
{
  return order_book[matched_fd].first.second;
}