#ifndef WIRE_H
#define WIRE_H

#include "message.h"

namespace Wire {

// Convert a ClientMode value to a string
std::string client_mode_to_str(ClientMode client_mode);

// Convert a string to a ClientMode value
ClientMode str_to_client_mode(const std::string &s);

// Convert a MessageType value to a string
std::string message_type_to_str(MessageType message_type);

// Convert a string to a MessageType value
MessageType str_to_message_type(const std::string &s);

// Convert an ItemStatus value to a string
std::string item_status_to_str(ItemStatus item_status);

// Convert a string to an ItemStatus value
ItemStatus str_to_item_status(const std::string &s);

// Convert an OrderStatus value to a string
std::string order_status_to_str(OrderStatus order_status);

// Convert a string to an OrderStatus value
OrderStatus str_to_order_status(const std::string &s);

// Encode the data in a given Message object as a string.
void encode(const Message &msg, std::string &s);

// Decode an encoded message from a string, using the decoded
// data to populate the given Message object. Throws InvalidMessage
// exception if the string does not contain a valid encoded
// message.
void decode(const std::string &s, Message &msg);

}

#endif // WIRE_H
