#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "Order.h"
#include "OrderBook.h"

class Session;

class OrderBookHelper
{
private:
    OrderBook& ptr_OrderBook;
    std::map<uint64_t,Session*>& m_sessions;
    struct MessageHeader msgheader;
    struct Exec execmessage;
    char temp_buffer[1000000]; //temporary buffer used for writing

    

public:
    OrderBookHelper(std::map<uint64_t,Session*>& sessions,OrderBook& ptr):m_sessions(sessions), ptr_OrderBook(ptr) {
     
     }

    void insert_in_queue(Order& order);
    void match_orders();

	~OrderBookHelper()
    {

    }


};