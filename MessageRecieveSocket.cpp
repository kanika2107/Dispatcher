#include "MessageRecieveSocket.h"
#include "Session.h"

std::map<uint64_t,int> MessageRecieveSocket::users;

 MessageRecieveSocket::MessageRecieveSocket(int fd,std::map<uint64_t,Session*>& sessions,time_t read_timeout, time_t write_timeout,OrderBookHelper& order_book_helper): m_sessions(sessions),ptr_OrderBookHelper(order_book_helper) {
    client_token=-1;
    m_fd = fd;
    no_bytes_read=0;
    was_header_read=false;
    was_message_read=false;
    msg_header = m_msg_header; 
    read_buffer = m_read_buffer;
    flag_has_exec_data=false;
    m_read_timeout = read_timeout;
    m_write_timeout = write_timeout;

  }

  Socket::HandlerResult MessageRecieveSocket::CheckForError(int bytes_read)
  {
      if (bytes_read < 0) {
       if (errno != EAGAIN) warn("read");
        return Socket::HandlerResult::NORMAL;
       } 
       else if (bytes_read == 0) {
        return CloseConnection();
      }
      return Socket::HandlerResult::NORMAL_CONTINUE;
  }

  void MessageRecieveSocket::PrepareMsgHeader(const MessageType& type,uint64_t seqnum,uint32_t size)
  {
    msgheader.type=type;
    msgheader.sequence_number=seqnum;
    msgheader.size = size;
  }

  Socket::HandlerResult MessageRecieveSocket::PartialReadMessageHeader()
  {
    ssize_t bytes_read;
    int iovcnt;
    if(was_header_read==false)
    {
       bytes_read=read(m_fd,msg_header+no_bytes_read,sizeof(MessageHeader)-no_bytes_read);
       Socket::HandlerResult result = CheckForError(bytes_read);
       if(result!=Socket::HandlerResult::NORMAL_CONTINUE)
        return result;

       no_bytes_read+=bytes_read;
       if(no_bytes_read==sizeof(MessageHeader))
       {
        was_header_read=true;
        no_bytes_read=0;
       }
     }
     return Socket::HandlerResult::NORMAL_CONTINUE;
  }

  Socket::HandlerResult MessageRecieveSocket::PartialReadMessageContents()
  {
    ssize_t bytes_read;
    int iovcnt;
    if(was_header_read==true)
    {

      std::cout << "Complete Header read" << std::endl;
      read_header = reinterpret_cast<MessageHeader*>(msg_header);

      //If message was logout, no need to read the contents as it is empty
      if(read_header->type==MessageType::LOGOUT)
      {
        
        was_message_read=true;
        return Socket::HandlerResult::NORMAL_CONTINUE;
      }

      bytes_read=read(m_fd,read_buffer+no_bytes_read,read_header->size-no_bytes_read);
      
      Socket::HandlerResult result = CheckForError(bytes_read);
      if(result!=Socket::HandlerResult::NORMAL_CONTINUE)
        return result;      
      
      no_bytes_read+=bytes_read;
      std::cout << read_header->size << " " << no_bytes_read << std::endl;
      if(no_bytes_read==read_header->size)
      {
        was_message_read=true;
        no_bytes_read=0;
      }
    }
    return Socket::HandlerResult::NORMAL_CONTINUE;
  }

   Socket::HandlerResult MessageRecieveSocket::HandleRead()  {

    //Read the message
    Socket::HandlerResult result = PartialReadMessageHeader();
    if(result==Socket::HandlerResult::NORMAL || result==Socket::HandlerResult::REMOVE)
      return result;
    result = PartialReadMessageContents();
    if(result==Socket::HandlerResult::NORMAL || result==Socket::HandlerResult::REMOVE)
      return result;
    
    if(was_message_read==true)
    {
      std::cout << "Complete Message read" << std::endl;
      was_message_read=false;
      was_header_read=false;

      if(read_header->type==MessageType::LOGIN)
        return LoginHandler(read_buffer); 

      else if(read_header->type==MessageType::PING)
        return PingHandler(read_buffer);

      else if(read_header->type==MessageType::NEW_ORDER)
        return NewOrderHandler(read_buffer);

      else if(read_header->type==MessageType::LOGOUT)
        return LogoutHandler();

    }
    return Socket::HandlerResult::NORMAL;
}
    
  Socket::HandlerResult MessageRecieveSocket::LogoutHandler()
  {
    std::cout << "Logout Called" << std::endl;
    return CloseConnection();
  }

  Socket::HandlerResult MessageRecieveSocket::PrepareHeartbeat()
  {
      std::cout << "Preparing Heartbeat from Timer callback" << std::endl;
      
      PrepareMsgHeader(MessageType::PING,1,sizeof(Ping));

      srand (time(NULL));
      pingmessage.cookie=rand()%100;
      std::cout << pingmessage.cookie << std::endl;
      
      //Append message header to the write buffer
      char *temp_buffer_pointer=temp_buffer;
      std::memcpy(temp_buffer,&msgheader,sizeof(msgheader));
      m_write_buffer.append(temp_buffer_pointer,sizeof(msgheader));

      //Append Ping message to the write buffer
      temp_buffer_pointer=temp_buffer;
      std::memcpy(temp_buffer,&pingmessage,sizeof(pingmessage));
      m_write_buffer.append(temp_buffer_pointer,sizeof(pingmessage)); 

      return Socket::HandlerResult::NORMAL;
  }

  Socket::HandlerResult MessageRecieveSocket::CloseConnection()
  {
      std::cout << "Closing client's connection" << std::endl;
      users.erase(client_token);
      m_sessions[this->client_token]->m_ptr_socket=nullptr;
      
      return Socket::HandlerResult::REMOVE; //Connection was closed.
  }

  void MessageRecieveSocket::AppendToWriteBuffer(const void* src,size_t size)
  {
      char *temp_buffer_pointer=temp_buffer;
      std::memcpy(temp_buffer,src,size);
      m_write_buffer.append(temp_buffer_pointer,size);
  }

  Socket::HandlerResult MessageRecieveSocket::LoginHandler(char* buffer) 
  {

      std::cout << "LoginHandler" << std::endl;
      Login* loginmessage = reinterpret_cast<Login*>(buffer);
      if(loginmessage->client_token==0 || loginmessage->client_token==1 || loginmessage->client_token==2) //Do validation on the UserID
      {
        
        if(users.count(loginmessage->client_token)!=0 && users[loginmessage->client_token]!=m_fd)
        {
          //Client already has an open connection so close this one
          std::cout << "Multiple client connections are not allowed" << std::endl;
          return Socket::HandlerResult::REMOVE;
        }
        if(users.count(loginmessage->client_token)!=0)
        {
          //Client already has logged in so ignore this message
          std::cout << "Client was already logged in" << std::endl;
          return Socket::HandlerResult::NORMAL;
        }

        //Register the client
        users[loginmessage->client_token] = m_fd;
        this->client_token = loginmessage->client_token;

        //Associate this client with this ssession
        m_sessions[this->client_token]->m_ptr_socket=this;
        //Append any buffers that were related to this session
        m_write_buffer.append(m_sessions[this->client_token]->buffer);
        m_sessions[this->client_token]->buffer.erase(0,m_sessions[this->client_token]->buffer.length());
        
        std::cout << "Logging you in" << std::endl;
        PrepareMsgHeader(MessageType::LOGIN_SUCCESS,1,0);
        AppendToWriteBuffer(&msgheader,sizeof(msgheader));        
      }
      else
      {
        std::cout << "Login client token is not valid" << std::endl; 
        PrepareMsgHeader(MessageType::LOGIN_FAILURE,1,0);  
        AppendToWriteBuffer(&msgheader,sizeof(msgheader));   
      }
      return Socket::HandlerResult::NORMAL;
  }

  Socket::HandlerResult MessageRecieveSocket::PingHandler(char* buffer)
  {

      //close the connection if client is not logged in
      if(users.count(client_token)==0)
        return Socket::HandlerResult::REMOVE;

      std::cout << "PingHandler" << std::endl;
      Ping* ping_message = reinterpret_cast<Ping*>(buffer);
      std::cout << ping_message->cookie << std::endl;
     
      //Prepare a Pong in response to the Ping
      PrepareMsgHeader(MessageType::PONG,1,sizeof(Pong));

      pongmessage.cookie=ping_message->cookie;
      
      //Append header to the write buffer
      AppendToWriteBuffer(&msgheader,sizeof(msgheader));
      
      //Append message to the write buffer
      AppendToWriteBuffer(&pongmessage,sizeof(pongmessage));
      
      return Socket::HandlerResult::NORMAL;
  }

  Socket::HandlerResult MessageRecieveSocket::PrepareNackMsg(NewOrder* new_order)
  {
      //Send Nacks
      std::cout << "Nack Message is being sent" << std::endl;

      PrepareMsgHeader(MessageType::ORDER_NACK,1,sizeof(Nack));
 
      nackmessage.order_id=new_order->order_id;
      
      //Append header to the write buffer
      AppendToWriteBuffer(&msgheader,sizeof(msgheader));

      //Append message to the write buffer
      AppendToWriteBuffer(&nackmessage,sizeof(nackmessage));

      return Socket::HandlerResult::NORMAL;
  }

  

  Socket::HandlerResult MessageRecieveSocket::PrepareAckMsg(NewOrder* new_order)
  {
      //Send an Ack
      std::cout << "Ack Message is being sent" << std::endl;

      PrepareMsgHeader(MessageType::ORDER_ACK,1,sizeof(Ack));
   
      ackmessage.order_id=new_order->order_id;
      
      //Append header to the write buffer 
      AppendToWriteBuffer(&msgheader,sizeof(msgheader)); 
      
      //Append message to the write buffer
      AppendToWriteBuffer(&ackmessage,sizeof(ackmessage));
      return Socket::HandlerResult::NORMAL;
  }


  Socket::HandlerResult MessageRecieveSocket::NewOrderHandler(char* buffer)
  {

    //close the connection if client is not logged in and sends some message
    if(users.count(client_token)==0)
        return Socket::HandlerResult::REMOVE;

    std::cout << "NewOrderRecieved" << std::endl;
    NewOrder* new_order=reinterpret_cast<NewOrder*>(buffer);
    
    if(new_order->symbol!=0)
      return PrepareNackMsg(new_order);
    
    //Keep inserting the orders in the queue as soon as they arrive
    Order my_order(new_order->price,new_order->qty,new_order->symbol,new_order->order_id,new_order->side,this->client_token,new_order->type,ExecType::COMPLETE_FILL);
    ptr_OrderBookHelper.insert_in_queue(my_order);

    return PrepareAckMsg(new_order);
    //ptr_OrderBookHelper.match_orders();


  }

  Socket::HandlerResult MessageRecieveSocket::HandleWrite() {

    if(!m_sessions[this->client_token]->buffer.empty())
    {
      m_write_buffer.append(m_sessions[this->client_token]->buffer);
      m_sessions[this->client_token]->buffer.erase(0,m_sessions[this->client_token]->buffer.length());
    }

    ssize_t res = write(m_fd, m_write_buffer.data(), m_write_buffer.size());

    std::cout << m_fd << " " << res << std::endl;
    if (res < 0)
    {
      if (errno != EAGAIN) warn("write");
      return Socket::HandlerResult::NORMAL;
    }
    m_write_buffer.erase(0, res);
    flag_has_exec_data=false;
    return Socket::HandlerResult::NORMAL;
  }

  bool MessageRecieveSocket::HasDataForWriting() {
    return !m_write_buffer.empty() || flag_has_exec_data;
  }