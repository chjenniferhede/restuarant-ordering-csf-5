#include <string>
#include <unordered_map>
#include <cassert>
#include <stdexcept>
#include "util.h"
#include "model.h"
#include "except.h"
#include "wire.h"

namespace {

// Private data and helper functions

const std::unordered_map<ClientMode, std::string> s_client_mode_to_str = {
  { ClientMode::INVALID, "INVALID" },
  { ClientMode::UPDATER, "UPDATER" },
  { ClientMode::DISPLAY, "DISPLAY" },
};

const std::vector<std::pair<std::string, ClientMode>> s_str_to_client_mode_vec =
  Util::invert_unordered_map(s_client_mode_to_str);

const std::unordered_map<std::string, ClientMode> s_str_to_client_mode(
  s_str_to_client_mode_vec.begin(),
  s_str_to_client_mode_vec.end()
);

const std::unordered_map<MessageType, std::string> s_message_type_to_str = {
  { MessageType::INVALID, "INVALID" },
  { MessageType::LOGIN, "LOGIN" },
  { MessageType::QUIT, "QUIT" },
  { MessageType::ORDER_NEW, "ORDER_NEW" },
  { MessageType::ITEM_UPDATE, "ITEM_UPDATE" },
  { MessageType::ORDER_UPDATE, "ORDER_UPDATE" },
  { MessageType::OK, "OK" },
  { MessageType::ERROR, "ERROR" },
  { MessageType::DISP_ORDER, "DISP_ORDER" },
  { MessageType::DISP_ITEM_UPDATE, "DISP_ITEM_UPDATE" },
  { MessageType::DISP_ORDER_UPDATE, "DISP_ORDER_UPDATE" },
  { MessageType::DISP_HEARTBEAT, "DISP_HEARTBEAT" },
};

const std::vector<std::pair<std::string, MessageType>> s_str_to_message_type_vec =
  Util::invert_unordered_map(s_message_type_to_str);

const std::unordered_map<std::string, MessageType> s_str_to_message_type(
  s_str_to_message_type_vec.begin(),
  s_str_to_message_type_vec.end()
);

const std::unordered_map<ItemStatus, std::string> s_item_status_to_str = {
  { ItemStatus::INVALID, "INVALID" },
  { ItemStatus::NEW, "NEW" },
  { ItemStatus::IN_PROGRESS, "IN_PROGRESS" },
  { ItemStatus::DONE, "DONE" },
};

const std::vector<std::pair<std::string, ItemStatus>> s_str_to_item_status_vec =
  Util::invert_unordered_map(s_item_status_to_str);

const std::unordered_map<std::string, ItemStatus> s_str_to_item_status(
  s_str_to_item_status_vec.begin(),
  s_str_to_item_status_vec.end()
);

const std::unordered_map<OrderStatus, std::string> s_order_status_to_str = {
  { OrderStatus::INVALID, "INVALID" },
  { OrderStatus::NEW, "NEW" },
  { OrderStatus::IN_PROGRESS, "IN_PROGRESS" },
  { OrderStatus::DONE, "DONE" },
  { OrderStatus::DELIVERED, "DELIVERED" },
};

const std::vector<std::pair<std::string, OrderStatus>> s_str_to_order_status_vec =
  Util::invert_unordered_map(s_order_status_to_str);

const std::unordered_map<std::string, OrderStatus> s_str_to_order_status(
  s_str_to_order_status_vec.begin(),
  s_str_to_order_status_vec.end()
);

// New private helper functions.
// Suggestion: functions to help encode and decode components
// of messages (in support of the Wire::encode and Wire::decode
// functions) would be a good idea.

// Parse a integer frome a string
int parse_positive_int(const std::string &str, const char *field_name) {
  size_t pos = 0;
  int number = -1;

  try {
    // stoi is parse string to int, and it sets pos to the index of the first character after the integer. 
    number = std::stoi(str, &pos, 10);
  } catch (std::exception &) {
    throw InvalidMessage(std::string("invalid integer field: ") + field_name);
  }

  // the whole str must be consumed, and the number must be positive
  if (pos != str.size() || number <= 0)
    throw InvalidMessage(std::string("invalid positive integer field: ") + field_name);

  return number;
}

// Turn a order into a string
// order_id,order_status,item_order_id:item_id:item_status:item_desc:item_qty;item_order_id:item_id:item_status:item_desc:item_qty;...
std::string encode_order(const std::shared_ptr<Order> &order) {
  if (!order || order->get_num_items() <= 0)
    throw InvalidMessage("invalid order payload");

  std::string out =
    std::to_string(order->get_id()) + "," +
    Wire::order_status_to_str(order->get_status()) + ",";

  // Add items, separated by ; each item has fields separated by :
  for (int i = 0; i < order->get_num_items(); ++i) {
    auto item = order->at(i);
    if (!item)
      throw InvalidMessage("invalid order item payload");

    if (i > 0)
      out += ";";

    out += std::to_string(item->get_order_id()) + ":" +
           std::to_string(item->get_id()) + ":" +
           Wire::item_status_to_str(item->get_status()) + ":" +
           item->get_desc() + ":" +
           std::to_string(item->get_qty());
  }

  return out;
}

// Turn a string into an order, check for formatting and validity. 
// string format: order_id,order_status,item_order_id:item_id:item_status:item_desc:item_qty;item_order_id:item_id:item_status:item_desc:item_qty;...
std::shared_ptr<Order> decode_order(const std::string &s) {
  // split by ,
  std::vector<std::string> fields = Util::split(s, ',');
  if (fields.size() != 3)
    throw InvalidMessage("invalid order field count");

  // Parse order id and status.
  int order_id = parse_positive_int(fields[0], "order id");
  OrderStatus order_status = Wire::str_to_order_status(fields[1]);
  if (order_status == OrderStatus::INVALID)
    throw InvalidMessage("invalid order status");

  // get order items, split by ;
  std::vector<std::string> item_fields = Util::split(fields[2], ';');
  if (item_fields.empty())
    throw InvalidMessage("order has no items");

  // Make order and add items to it.
  auto order = std::make_shared<Order>(order_id, order_status);
  for (auto i = item_fields.begin(); i != item_fields.end(); ++i) {
    // one item's fields are split by :, in the format item_order_id:item_id:item_status:item_desc:item_qty
    std::vector<std::string> parts = Util::split(*i, ':');
    if (parts.size() != 5)
      throw InvalidMessage("invalid item field count");

    int item_order_id = parse_positive_int(parts[0], "item order id");
    int item_id = parse_positive_int(parts[1], "item id");
    ItemStatus item_status = Wire::str_to_item_status(parts[2]);
    if (item_status == ItemStatus::INVALID)
      throw InvalidMessage("invalid item status");
    int qty = parse_positive_int(parts[4], "item quantity");

    order->add_item(
      std::make_shared<Item>(item_order_id, item_id, item_status, parts[3], qty)
    );
  }

  return order;
}


} // end of anonymous namespace for helper functions

namespace Wire {

std::string client_mode_to_str(ClientMode client_mode) {
  // Map enum to protocol text.
  auto i = s_client_mode_to_str.find(client_mode);
  return (i != s_client_mode_to_str.end()) ? i->second : "<invalid>";
}

ClientMode str_to_client_mode(const std::string &s) {
  // Map protocol text to enum.
  auto i = s_str_to_client_mode.find(s);
  return (i != s_str_to_client_mode.end()) ? i->second : ClientMode::INVALID;
}

std::string message_type_to_str(MessageType message_type) {
  // Map enum to protocol text.
  auto i = s_message_type_to_str.find(message_type);
  return (i != s_message_type_to_str.end()) ? i->second : "<invalid>";
}

MessageType str_to_message_type(const std::string &s) {
  // Map protocol text to enum.
  auto i = s_str_to_message_type.find(s);
  return (i != s_str_to_message_type.end()) ? i->second : MessageType::INVALID;
}

std::string item_status_to_str(ItemStatus item_status) {
  // Map enum to protocol text.
  auto i = s_item_status_to_str.find(item_status);
  return (i != s_item_status_to_str.end()) ? i->second : "<invalid>";
}

ItemStatus str_to_item_status(const std::string &s) {
  // Map protocol text to enum.
  auto i = s_str_to_item_status.find(s);
  return (i != s_str_to_item_status.end()) ? i->second : ItemStatus::INVALID;
}

std::string order_status_to_str(OrderStatus order_status) {
  // Map enum to protocol text.
  auto i = s_order_status_to_str.find(order_status);
  return (i != s_order_status_to_str.end()) ? i->second : "<invalid>";
}

OrderStatus str_to_order_status(const std::string &s) {
  // Map protocol text to enum.
  auto i = s_str_to_order_status.find(s);
  return (i != s_str_to_order_status.end()) ? i->second : OrderStatus::INVALID;
}

// Given a message object, format it into a string
void encode(const Message &msg, std::string &result_str) {
  
  // get the type, route to ecoding logic 
  MessageType type = msg.get_type();
  switch (type) {
  case MessageType::LOGIN:
    if (!msg.has_client_mode() || !msg.has_str())
      throw InvalidMessage("invalid LOGIN message");
    result_str = "LOGIN|" + client_mode_to_str(msg.get_client_mode()) + "|" + msg.get_str();
    return;

  case MessageType::QUIT:
    if (!msg.has_str())
      throw InvalidMessage("invalid QUIT message");
    result_str = "QUIT|" + msg.get_str();
    return;

  case MessageType::ORDER_NEW:
    result_str = "ORDER_NEW|" + encode_order(msg.get_order());
    return;

  case MessageType::ITEM_UPDATE:
    if (!msg.has_order_id() || !msg.has_item_id() || !msg.has_item_status())
      throw InvalidMessage("invalid ITEM_UPDATE message");
    result_str = "ITEM_UPDATE|" +
      std::to_string(msg.get_order_id()) + "|" +
      std::to_string(msg.get_item_id()) + "|" +
      item_status_to_str(msg.get_item_status());
    return;

  case MessageType::ORDER_UPDATE:
    if (!msg.has_order_id() || !msg.has_order_status())
      throw InvalidMessage("invalid ORDER_UPDATE message");
    result_str = "ORDER_UPDATE|" +
      std::to_string(msg.get_order_id()) + "|" +
      order_status_to_str(msg.get_order_status());
    return;

  case MessageType::OK:
    if (!msg.has_str())
      throw InvalidMessage("invalid OK message");
    result_str = "OK|" + msg.get_str();
    return;

  case MessageType::ERROR:
    if (!msg.has_str())
      throw InvalidMessage("invalid ERROR message");
    result_str = "ERROR|" + msg.get_str();
    return;

  case MessageType::DISP_ORDER:
    result_str = "DISP_ORDER|" + encode_order(msg.get_order());
    return;

  case MessageType::DISP_ITEM_UPDATE:
    if (!msg.has_order_id() || !msg.has_item_id() || !msg.has_item_status())
      throw InvalidMessage("invalid DISP_ITEM_UPDATE message");
    result_str = "DISP_ITEM_UPDATE|" +
      std::to_string(msg.get_order_id()) + "|" +
      std::to_string(msg.get_item_id()) + "|" +
      item_status_to_str(msg.get_item_status());
    return;

  case MessageType::DISP_ORDER_UPDATE:
    if (!msg.has_order_id() || !msg.has_order_status())
      throw InvalidMessage("invalid DISP_ORDER_UPDATE message");
    result_str = "DISP_ORDER_UPDATE|" +
      std::to_string(msg.get_order_id()) + "|" +
      order_status_to_str(msg.get_order_status());
    return;

  case MessageType::DISP_HEARTBEAT:
    result_str = "DISP_HEARTBEAT";
    return;

  default:
    throw InvalidMessage("invalid message type for encoding");
  }
}

// Given a string, parse it into a message object
void decode(const std::string &given_str, Message &msg) {

  // Parse one wire-format string into a validated Message object.
  std::vector<std::string> overall_fields = Util::split(given_str, '|');
  if (overall_fields.empty())
    throw InvalidMessage("empty message");

  // get the msg type
  MessageType type = str_to_message_type(overall_fields[0]);
  if (type == MessageType::INVALID)
    throw InvalidMessage("invalid message type");

  // route to decoding logic based on type
  switch (type) {
  case MessageType::LOGIN: {
    if (overall_fields.size() != 3)
      throw InvalidMessage("invalid LOGIN field count");
    ClientMode mode = str_to_client_mode(overall_fields[1]);
    if (mode == ClientMode::INVALID)
      throw InvalidMessage("invalid client mode");
    msg = Message(type, mode, overall_fields[2]);
    return;
  }

  // Quite, Ok, Error all have two fields: type and a string field. 
  case MessageType::QUIT:
  case MessageType::OK:
  case MessageType::ERROR:
    if (overall_fields.size() != 2)
      throw InvalidMessage("invalid string message field count");
    msg = Message(type, overall_fields[1]);
    return;

  // Order new and disp order have two fields: type and an order field.
  case MessageType::ORDER_NEW:
  case MessageType::DISP_ORDER:
    if (overall_fields.size() != 2)
      throw InvalidMessage("invalid order message field count");
    msg = Message(type, decode_order(overall_fields[1]));
    return;

  // Item update and order update have four same fields: type, order id, item id (for item update), and status.
  case MessageType::ITEM_UPDATE:
  case MessageType::DISP_ITEM_UPDATE: {
    if (overall_fields.size() != 4)
      throw InvalidMessage("invalid item update field count");

    int order_id = parse_positive_int(overall_fields[1], "order id");
    int item_id = parse_positive_int(overall_fields[2], "item id");
    ItemStatus item_status = str_to_item_status(overall_fields[3]);

    if (item_status == ItemStatus::INVALID)
      throw InvalidMessage("invalid item status");

    // build the message
    msg = Message(type, order_id, item_id, item_status);
    return;
  }

  // Both Order update has three fields: type, order id, and order status.
  case MessageType::ORDER_UPDATE:
  case MessageType::DISP_ORDER_UPDATE: {
    if (overall_fields.size() != 3)
      throw InvalidMessage("invalid order update field count");
    int order_id = parse_positive_int(overall_fields[1], "order id");
    OrderStatus order_status = str_to_order_status(overall_fields[2]);
    if (order_status == OrderStatus::INVALID)
      throw InvalidMessage("invalid order status");
    msg = Message(type, order_id, order_status);
    return;
  }

  case MessageType::DISP_HEARTBEAT:
    if (overall_fields.size() != 1)
      throw InvalidMessage("invalid heartbeat field count");
    msg = Message(type);
    return;

  default:
    throw InvalidMessage("invalid message type for decoding");
  }
}

}
