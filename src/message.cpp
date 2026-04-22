#include "message.h"

Message::Message()
  : m_type(MessageType::INVALID)
  , m_client_mode(ClientMode::INVALID)
  , m_order_id(-1)
  , m_order_status(OrderStatus::INVALID)
  , m_item_id(-1)
  , m_item_status(ItemStatus::INVALID) {
}

Message::Message(const Message &other)
  : Message() {
  *this = other;
}

Message::Message(MessageType type)
  : Message() {
  m_type = type;
}

Message::Message(MessageType type, const std::string &str)
  : Message() {
  m_type = type;
  m_str = str;
}

Message::Message(MessageType type, ClientMode client_mode, const std::string &str)
  : Message() {
  m_type = type;
  m_client_mode = client_mode;
  m_str = str;
}

Message::Message(MessageType type, std::shared_ptr<Order> order)
  : Message() {
  m_type = type;
  m_order = order;
}

Message::Message(MessageType type, int order_id, OrderStatus order_status)
  : Message() {
  m_type = type;
  m_order_id = order_id;
  m_order_status = order_status;
}

Message::Message(MessageType type, int order_id, int item_id, ItemStatus item_status)
  : Message() {
  m_type = type;
  m_order_id = order_id;
  m_item_id = item_id;
  m_item_status = item_status;
}

Message::~Message() {
}

Message &Message::operator=(const Message &rhs) {
  if (this != &rhs) {
    m_type = rhs.m_type;
    m_client_mode = rhs.m_client_mode;
    m_str = rhs.m_str;
    if (rhs.m_order)
      m_order = rhs.m_order->duplicate(); // make a deep copy
    else
      m_order.reset();
    m_order_id = rhs.m_order_id;
    m_order_status = rhs.m_order_status;
    m_item_id = rhs.m_item_id;
    m_item_status = rhs.m_item_status;
  }
  return *this;
}

std::shared_ptr<Message> Message::duplicate() const {
  return std::make_shared<Message>(*this);
}

bool Message::operator==(const Message &rhs) const {
  if (get_type() != rhs.get_type() ||
      get_client_mode() != rhs.get_client_mode() ||
      get_str() != rhs.get_str() ||
      (has_order() && (!rhs.has_order() || *get_order() != *rhs.get_order())) ||
      get_order_id() != rhs.get_order_id() ||
      get_order_status() != rhs.get_order_status() ||
      get_item_id() != rhs.get_item_id() ||
      get_item_status() != rhs.get_item_status())
    return false;
  else
    return true;
}
