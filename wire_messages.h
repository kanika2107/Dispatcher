#ifndef WIRE_MESSAGES_H__
#define WIRE_MESSAGES_H__

#include <cstdint>

enum class Side : uint8_t {
  BUY,
  SELL,
};

enum class MessageType : int32_t {
  LOGIN,
  LOGIN_FAILURE,
  LOGIN_SUCCESS,
  NEW_ORDER,
  ORDER_ACK,
  ORDER_NACK,
  EXEC,
  PING,
  PONG,
};

using order_id_t = uint32_t;
using seq_t = uint64_t;

struct MessageHeader {
  MessageType type;
  uint32_t size;
  seq_t sequence_number;
};

struct NewOrder {
  double price;
  uint32_t qty;
  uint32_t symbol;
  order_id_t order_id;
  Side side;
} __attribute__((packed));

//static_assert(sizeof(OrderMessage) == 21, "Problem with stucture layout");

struct Ack {
  order_id_t order_id;
};

struct Nack {
  order_id_t order_id;
};

struct Exec {
  order_id_t order_id;
  double price;
  uint32_t qty;
  uint32_t symbol;
  Side side;
};

struct Login {
  uint64_t client_token;
  seq_t last_seqnum;
  static constexpr seq_t RESET_SEQUENCE_NUMBERS = -1;
};

using heartbeat_t = uint32_t;
struct Ping {
  heartbeat_t cookie;
};

struct Pong {
  heartbeat_t cookie;
};

#endif // WIRE_MESSAGES_H__
