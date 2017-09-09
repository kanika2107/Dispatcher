#pragma once

#include "wire_messages.h"

struct Order
{
	double price;
	uint32_t qty;
	uint32_t symbol;
	uint32_t order_id;
    Side side;
    uint64_t client_token;
    NewOrderType type;
    ExecType exec_type;

    Order(double price,uint32_t qty, uint32_t symbol, uint32_t order_id, Side side, uint64_t client_token, NewOrderType type, ExecType exec_type ): price(price),qty(qty), symbol(symbol), order_id(order_id), side(side), client_token(client_token), type(type), exec_type(exec_type) {}

};
