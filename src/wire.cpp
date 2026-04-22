#include <string>
#include <unordered_map>
#include <cassert>
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

// TODO: add private helper functions.
// Suggestion: functions to help encode and decode components
// of messages (in support of the Wire::encode and Wire::decode
// functions) would be a good idea.


} // end of anonymous namespace for helper functions

namespace Wire {

std::string client_mode_to_str(ClientMode client_mode) {
  auto i = s_client_mode_to_str.find(client_mode);
  return (i != s_client_mode_to_str.end()) ? i->second : "<invalid>";
}

ClientMode str_to_client_mode(const std::string &s) {
  auto i = s_str_to_client_mode.find(s);
  return (i != s_str_to_client_mode.end()) ? i->second : ClientMode::INVALID;
}

std::string message_type_to_str(MessageType message_type) {
  auto i = s_message_type_to_str.find(message_type);
  return (i != s_message_type_to_str.end()) ? i->second : "<invalid>";
}

MessageType str_to_message_type(const std::string &s) {
  auto i = s_str_to_message_type.find(s);
  return (i != s_str_to_message_type.end()) ? i->second : MessageType::INVALID;
}

std::string item_status_to_str(ItemStatus item_status) {
  auto i = s_item_status_to_str.find(item_status);
  return (i != s_item_status_to_str.end()) ? i->second : "<invalid>";
}

ItemStatus str_to_item_status(const std::string &s) {
  auto i = s_str_to_item_status.find(s);
  return (i != s_str_to_item_status.end()) ? i->second : ItemStatus::INVALID;
}

std::string order_status_to_str(OrderStatus order_status) {
  auto i = s_order_status_to_str.find(order_status);
  return (i != s_order_status_to_str.end()) ? i->second : "<invalid>";
}

OrderStatus str_to_order_status(const std::string &s) {
  auto i = s_str_to_order_status.find(s);
  return (i != s_str_to_order_status.end()) ? i->second : OrderStatus::INVALID;
}

void encode(const Message &msg, std::string &s) {
  // TODO: implement
}

void decode(const std::string &s, Message &msg) {
  // TODO: implement
}

}
