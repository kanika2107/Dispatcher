#pragma once

#include "MessageRecieveSocket.h"

#include <cstring>

//Each Session consists of a buffer of older execs and pointer to the Socket connection object that this session points to

class Session
{
public:
    MessageRecieveSocket* m_ptr_socket;
    std::string buffer;
    Session()
    {
         m_ptr_socket=nullptr;
    }
};


