#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "Order.h"

class OrderBook
{
private:
	std::vector<Order> order_queue;
	std::map<uint32_t, std::vector<Order> > buy_book;
	std::map<uint32_t, std::vector<Order> > sell_book;

    

public:

	OrderBook() { }
	void insert_order_in_queue(Order& order);
	void shuffle_orders_in_queue();
	void printQueueState() const ;
	void printBookState();
	int FindSELLInsertPosition(const Order& order);
	int FindBUYInsertPosition(const Order& order);
	std::vector<Order> FindMatches();
	std::vector<Order> match_orders();
	std::vector<Order> FindMarketOrderMatches(Order& order);
	~OrderBook()
    {
    }


};