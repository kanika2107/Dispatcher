#include "OrderBook.h"
#include "wire_messages.h"

#include <algorithm>    // std::random_shuffle 
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand


void OrderBook::printQueueState() const
{
	for(int i=0;i<order_queue.size();i++)
	{
		std::cout << order_queue[i].price << " " << order_queue[i].qty << " " << order_queue[i].symbol << " " << order_queue[i].order_id << " " << order_queue[i].client_token << std::endl;
	}

}

void OrderBook::insert_order_in_queue(Order& order)
{
	order_queue.push_back(order);
}

void OrderBook::shuffle_orders_in_queue()
{
	std::srand(unsigned(std::time(0)));
	std::random_shuffle(order_queue.begin(), order_queue.end());
}

int OrderBook::FindBUYInsertPosition(const Order& order)
{
  int index=buy_book[order.symbol].size();
  for(int i=0;i<buy_book[order.symbol].size();i++)
  {
    if(order.price>buy_book[order.symbol][i].price)
    {
      index=i;
      break;
    }

  }
  return index;

}

int OrderBook::FindSELLInsertPosition(const Order& order) 
{
  int index=sell_book[order.symbol].size();
  for(int i=0;i<sell_book[order.symbol].size();i++)
  {
    if(order.price<sell_book[order.symbol][i].price)
    {
      index=i;
      break;
    }

  }
  return index;

}

void OrderBook::printBookState()
{
  for(auto& pair: buy_book)
  {
    std::cout << "OrderBook for " << pair.first << std::endl;
    for(auto &item: buy_book[pair.first])
    {
      std::cout << item.price << " " << item.qty << " " << item.order_id << " " << item.client_token << std::endl; 
    }
  }

  for(auto& pair: sell_book)
  {
    std::cout << "OrderBook for " << pair.first << std::endl;
    for(auto &item: sell_book[pair.first])
    {
      std::cout << item.price << " " << item.qty << " " << item.order_id << " " << item.client_token << std::endl; 
    }
  }

}

std::vector<Order> OrderBook::FindMatches()
{
  std::vector<Order> result;
  for(auto& pair: buy_book)
  {
    if(!sell_book[pair.first].empty())
    {
      while(!sell_book[pair.first].empty() && !buy_book[pair.first].empty() && buy_book[pair.first][0].price>=sell_book[pair.first][0].price)
      {
        uint32_t matched_qty = std::min(buy_book[pair.first][0].qty,sell_book[pair.first][0].qty);
        ExecType type = ExecType::PARTIAL_FILL;
        if(matched_qty==buy_book[pair.first][0].qty)
                  type = ExecType::COMPLETE_FILL;
        Order buy_order(buy_book[pair.first][0].price,matched_qty,pair.first,buy_book[pair.first][0].order_id,Side::BUY,buy_book[pair.first][0].client_token,buy_book[pair.first][0].type,type);
        
        if(matched_qty==sell_book[pair.first][0].qty)
          type=ExecType::COMPLETE_FILL;
        else
          type=ExecType::PARTIAL_FILL;

        Order sell_order(sell_book[pair.first][0].price,matched_qty,pair.first,sell_book[pair.first][0].order_id,Side::SELL,sell_book[pair.first][0].client_token,sell_book[pair.first][0].type,type);
        buy_book[pair.first][0].qty-=matched_qty;
        sell_book[pair.first][0].qty-=matched_qty;
        result.push_back(buy_order);
        result.push_back(sell_order);
        if(buy_book[pair.first][0].qty==0)
          buy_book[pair.first].erase(buy_book[pair.first].begin());
        if(sell_book[pair.first][0].qty==0)
          sell_book[pair.first].erase(sell_book[pair.first].begin());
      }
    }
  }
  return result;
}

std::vector<Order> OrderBook::FindMarketOrderMatches(Order& order)
{
  std::vector<Order> result;
  if(order.side==Side::BUY)
  {
      while(!sell_book[order.symbol].empty() && order.qty!=0)
      {

        ExecType type=ExecType::PARTIAL_FILL;

        uint32_t matched_qty = std::min(order.qty,sell_book[order.symbol][0].qty);
        if(order.qty==matched_qty)
          type=ExecType::COMPLETE_FILL;

        Order buy_order(sell_book[order.symbol][0].price,matched_qty,order.symbol,order.order_id,order.side,order.client_token,order.type,type);

        if(sell_book[order.symbol][0].qty==matched_qty)
          type=ExecType::COMPLETE_FILL;
        else
          type=ExecType::PARTIAL_FILL;
        Order sell_order(sell_book[order.symbol][0].price,matched_qty,order.symbol,sell_book[order.symbol][0].order_id,Side::SELL,sell_book[order.symbol][0].client_token,sell_book[order.symbol][0].type,type);
        result.push_back(buy_order);
        result.push_back(sell_order);
        sell_book[order.symbol][0].qty-=matched_qty;
        order.qty-=matched_qty;
        if(sell_book[order.symbol][0].qty==0)
          sell_book[order.symbol].erase(sell_book[order.symbol].begin());
      }
  }
  else
  {
    while(!buy_book[order.symbol].empty() && order.qty!=0)
      {
        uint32_t matched_qty = std::min(order.qty,buy_book[order.symbol][0].qty);
        ExecType type=ExecType::  PARTIAL_FILL;
        if(order.qty==matched_qty)
          type=ExecType::COMPLETE_FILL;
        Order sell_order(buy_book[order.symbol][0].price,matched_qty,order.symbol,order.order_id,order.side,order.client_token,order.type,type);
        
         if(buy_book[order.symbol][0].qty==matched_qty)
          type=ExecType::COMPLETE_FILL;
        else
          type=ExecType::PARTIAL_FILL;

        Order buy_order(buy_book[order.symbol][0].price,matched_qty,order.symbol,buy_book[order.symbol][0].order_id,Side::BUY,buy_book[order.symbol][0].client_token,buy_book[order.symbol][0].type,type);
       
        result.push_back(buy_order);
        result.push_back(sell_order);
        buy_book[order.symbol][0].qty-=matched_qty;
        order.qty-=matched_qty;
        if(buy_book[order.symbol][0].qty==0)
          buy_book[order.symbol].erase(buy_book[order.symbol].begin());
      }

  }
  if(order.qty!=0)
  {
    //reject this amount
    ExecType type=ExecType::REJECT;
    Order sell_order(0,order.qty,order.symbol,order.order_id,order.side,order.client_token,order.type,type);
    result.push_back(sell_order);
  }

  return result;
}
//TODO
std::vector<Order> OrderBook::match_orders()
{
	//first shuffle the orders in the queue
	//shuffle_orders_in_queue();
  int index;
  std::vector<Order> matched_orders;
	for(int i=0;i<order_queue.size();i++)
  {    
    if(order_queue[i].side==Side::BUY)
    {
        if(order_queue[i].type==NewOrderType::MARKET_ORDER)
        {
          std::vector<Order> partial_result = FindMarketOrderMatches(order_queue[i]);
          matched_orders.insert(matched_orders.end(),partial_result.begin(),partial_result.end());
          continue;
        }
        index=FindBUYInsertPosition(order_queue[i]);
        buy_book[order_queue[i].symbol].insert(buy_book[order_queue[i].symbol].begin()+index, order_queue[i]);
    }
    else
    {
        if(order_queue[i].type==NewOrderType::MARKET_ORDER)
        {
          std::vector<Order> partial_result = FindMarketOrderMatches(order_queue[i]);
          matched_orders.insert(matched_orders.end(),partial_result.begin(),partial_result.end());
          continue;
        }
      index=FindSELLInsertPosition(order_queue[i]);
      sell_book[order_queue[i].symbol].insert(sell_book[order_queue[i].symbol].begin()+index, order_queue[i]);
    }
    std::vector<Order> partial_result = FindMatches();
    matched_orders.insert(matched_orders.end(),partial_result.begin(),partial_result.end());
  }
  
  while(!order_queue.empty())
  {
    order_queue.pop_back();
  }
  printBookState();
	return matched_orders;
}

/*int main()
{
  std::vector<Order> test_vec;
  test_vec.push_back(Order(33.75,200,0,101,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.74,500,0,102,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.78,300,0,103,Side::SELL,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.75,100,0,104,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.77,200,0,105,Side::SELL,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.74,500,0,106,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.73,700,0,107,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.77,400,0,108,Side::SELL,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.78,900,0,109,Side::SELL,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.763,500,0,112,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.751,100,0,113,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.762,300,0,114,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.76,900,0,115,Side::SELL,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.78,100,0,116,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.78,2000,0,117,Side::BUY,1,NewOrderType::LIMIT_ORDER,ExecType::COMPLETE_FILL));
  test_vec.push_back(Order(33.78,2600,0,118,Side::SELL,1,NewOrderType::MARKET_ORDER,ExecType::COMPLETE_FILL));

  for(int i=0;i<test_vec.size();i++)
  {
    OrderBook::getInstance()->insert_order_in_queue(test_vec[i]);
  }
  OrderBook::getInstance()->printQueueState();
  std::vector<Order> matched_orders = OrderBook::getInstance()->match_orders();
    std::cout << "MATCHED ORDERS " << std::endl;
  for(int i=0;i<matched_orders.size();i++)
  {
    std::cout << matched_orders[i].price << " " << matched_orders[i].qty << " " << matched_orders[i].symbol << " " << matched_orders[i].order_id << " " << matched_orders[i].client_token << std::endl;
  }
}*/
