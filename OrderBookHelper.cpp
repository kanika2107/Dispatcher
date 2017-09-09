#include "OrderBookHelper.h"

#include <algorithm>    // std::random_shuffle 
#include <iostream>
#include <map>
#include <memory>
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include <vector>
#include "Session.h"


void OrderBookHelper::insert_in_queue(Order& order)
{
    ptr_OrderBook.insert_order_in_queue(order);
}
    
void OrderBookHelper::match_orders()
{
    std::cout << "i am called" << std::endl;
    std::vector<Order> exec_orders = ptr_OrderBook.match_orders();
    for(int i=0;i<exec_orders.size();i++)
    {
      uint64_t client_token = exec_orders[i].client_token;
      //m_sessions[client_token]->m_ptr_socket->PrepareExecMsg(exec_orders[i]);
       std::cout << "EXEC Message is being prepared" << std::endl;
       std::cout << exec_orders.size() << std::endl;
      //send an EXEC for this order
      
      // MessageHeader msgheader;
      msgheader.type=MessageType::EXEC;
      msgheader.sequence_number = 2;
      msgheader.size=sizeof(Exec);

      // Exec execmessage;
      execmessage.order_id=exec_orders[i].order_id;
      execmessage.qty=exec_orders[i].qty;
      execmessage.price=exec_orders[i].price;
      execmessage.symbol=exec_orders[i].symbol;
      execmessage.side=exec_orders[i].side;
      execmessage.type=exec_orders[i].exec_type;


      //Append header to the write buffer 
      char *temp_buffer_pointer=temp_buffer;
      std::memcpy(temp_buffer,&msgheader,sizeof(msgheader));
     // std::cout << m_sessions[client_token]->buffer.length() << std::endl;
      m_sessions[client_token]->buffer.append(temp_buffer_pointer,sizeof(msgheader));
      
      //Append message to the write buffer
      temp_buffer_pointer=temp_buffer;
      std::memcpy(temp_buffer,&execmessage,sizeof(execmessage));
      m_sessions[client_token]->buffer.append(temp_buffer_pointer,sizeof(execmessage)); 

      std::cout << sizeof(execmessage)+sizeof(msgheader) << "Sizes" << std::endl;
      if(m_sessions[client_token]->m_ptr_socket!=nullptr)
      {
        m_sessions[client_token]->m_ptr_socket->flag_has_exec_data=true;
        
      }
    }
    //Do processing on the map of sessions
}
