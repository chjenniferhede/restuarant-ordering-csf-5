#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>
#include "model.h"

enum class MessageType {
  // Invalid message type
  INVALID,

  // ----------------------------------------------------------------------
  // Client messages ("requests", sent from client to server)
  // ----------------------------------------------------------------------

  // All clients
  LOGIN,                           // request to log in

  // Updater only
  QUIT,                            // client is closing connection
  ORDER_NEW,                       // a new order has been created
  ITEM_UPDATE,                     // status of Item is updated
  ORDER_UPDATE,                    // status of Order is updated

  // ----------------------------------------------------------------------
  // Server messages ("responses", sent from server to client)
  // ----------------------------------------------------------------------

  // All clients
  OK,                              // request was accepted
  ERROR,                           // request was not accepted

  // Display only
  DISP_ORDER,                      // initial data of an Order
  DISP_ITEM_UPDATE,                // status of an Item has been updated
  DISP_ORDER_UPDATE,               // status of an Order has been updated
  DISP_HEARTBEAT,                  // check whether the client is still running
};

// A Message is a MessageType and one of the following:
//   - nothing
//   - a string
//   - a ClientMode and a string
//   - a shared_ptr to an Order
//   - an Order id and an OrderStatus
//   - an Order id, an Item id, and an ItemStatus
//
// Note that there is a constructor for each of the above combinations
// of data values.
//
// Also note that Message objects do have value semantics
// (copying and assignment.)
//
// In a proper object-oriented design, we would have various
// subclasses for the different kinds of messages. For simplicity,
// we have a single Message class whose fields are the union of
// the data required for all message types.
class Message {
private:
  MessageType m_type;
  ClientMode m_client_mode;
  std::string m_str;
  std::shared_ptr<Order> m_order;
  int m_order_id;
  OrderStatus m_order_status;
  int m_item_id;
  ItemStatus m_item_status;

public:
  Message(); // sets all fields to an empty/invalid state
  Message(const Message &other);
  Message(MessageType type);
  Message(MessageType type, const std::string &str);
  Message(MessageType type, ClientMode client_mode, const std::string &str);
  Message(MessageType type, std::shared_ptr<Order> order);
  Message(MessageType type, int order_id, OrderStatus order_status);
  Message(MessageType type, int order_id, int item_id, ItemStatus item_status);
  ~Message();

  Message &operator=(const Message &rhs);

  // Return a std::shared_ptr to an exact duplicate of this Message.
  // This is useful for making a copy of a message to broadcast to
  // a DISPLAY client.
  std::shared_ptr<Message> duplicate() const;

  bool operator==(const Message &rhs) const;

  MessageType get_type() const { return m_type; }
  void set_type(MessageType type) { m_type = type; }

  bool has_client_mode() const { return m_client_mode != ClientMode::INVALID; }
  ClientMode get_client_mode() const { return m_client_mode; }
  void set_client_mode(ClientMode client_mode) { m_client_mode = client_mode; }

  bool has_str() const { return !m_str.empty(); }
  const std::string &get_str() const { return m_str; }
  void set_str(const std::string &str) { m_str = str; }

  bool has_order() const { return bool(m_order); }
  std::shared_ptr<Order> get_order() const { return m_order; }
  void set_order(std::shared_ptr<Order> order) { m_order = order; }

  bool has_order_id() const { return m_order_id >= 0; }
  int get_order_id() const { return m_order_id; }
  void set_order_id(int order_id) { m_order_id = order_id; }

  bool has_order_status() const { return m_order_status != OrderStatus::INVALID; }
  OrderStatus get_order_status() const { return m_order_status; }
  void set_order_status(OrderStatus order_status) { m_order_status = order_status; }

  bool has_item_id() const { return m_item_id >= 0; }
  int get_item_id() const { return m_item_id; }
  void set_item_id(int item_id) { m_item_id = item_id; }

  bool has_item_status() const { return m_item_status != ItemStatus::INVALID; }
  ItemStatus get_item_status() const { return m_item_status; }
  void set_item_status(ItemStatus item_status) { m_item_status = item_status; }
};

#endif // MESSAGE_H
