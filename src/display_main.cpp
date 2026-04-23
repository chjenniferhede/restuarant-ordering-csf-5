// Main program for the display client

#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include "csapp.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "util.h"
#include "client_util.h"
#include "fd_guard.h"

namespace {

// This terminal escape sequence should be written to
// std::cout before the display client redisplays the current
// order information.
const char CLEAR_SCREEN[] = "\x1b[2J\x1b[H";

void print_order(const std::shared_ptr<Order> &order) {
  std::cout << "Order " << order->get_id() << ": "
            << Wire::order_status_to_str(order->get_status()) << "\n";

  for (int i = 0; i < order->get_num_items(); ++i) {
    auto item = order->at(i);
    std::cout << "  Item " << item->get_id() << ": "
              << Wire::item_status_to_str(item->get_status()) << "\n";
    std::cout << "    " << item->get_desc() << ", Quantity " << item->get_qty() << "\n";
  }
}

// clear the display and print the current orders
void refresh_display(const std::map<int, std::shared_ptr<Order>> &orders) {
  std::cout << CLEAR_SCREEN << std::flush;

  for (auto i = orders.begin(); i != orders.end(); ++i)
    print_order(i->second);
}

// send and receive
Message send_request_receive_response(int fd, const Message &request) {
  ClientUtil::send_message(fd, request);

  Message response;
  ClientUtil::receive_message(fd, response);
  return response;
}

// handle a DISP_ORDER message: add the new order to the orders data structure
void handle_disp_order(
  const Message &msg,
  std::map<int, std::shared_ptr<Order>> &orders
) {
  if (!msg.has_order())
    throw ProtocolError("missing order in DISP_ORDER");

  auto order = msg.get_order();
  orders[order->get_id()] = order->duplicate();
}

// handle a DISP_ITEM_UPDATE message: update the status of the specified item
void handle_disp_item_update(
  const Message &msg,
  std::map<int, std::shared_ptr<Order>> &orders
) {
  // check that the message has the required fields
  if (!msg.has_order_id() || !msg.has_item_id() || !msg.has_item_status())
    throw ProtocolError("missing fields in DISP_ITEM_UPDATE");

  // find the order and item specified in the message
  auto order_i = orders.find(msg.get_order_id());
  if (order_i == orders.end())
    throw ProtocolError("unknown order id in DISP_ITEM_UPDATE");

  // find the item in the order and update its status
  auto item = order_i->second->find_item(msg.get_item_id());
  if (!item)
    throw ProtocolError("unknown item id in DISP_ITEM_UPDATE");

  item->set_status(msg.get_item_status());
}

// handle a DISP_ORDER_UPDATE message: update the status of the specified order
void handle_disp_order_update(
  const Message &msg,
  std::map<int, std::shared_ptr<Order>> &orders
) {
  if (!msg.has_order_id() || !msg.has_order_status())
    throw ProtocolError("missing fields in DISP_ORDER_UPDATE");

  int order_id = msg.get_order_id();
  OrderStatus order_status = msg.get_order_status();

  auto order_i = orders.find(order_id);
  if (order_i == orders.end())
    throw ProtocolError("unknown order id in DISP_ORDER_UPDATE");

  if (order_status == OrderStatus::DELIVERED) {
    orders.erase(order_i);
    return;
  }

  order_i->second->set_status(order_status);
}

}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <hostname> <port>\n";
    return 1;
  }

  try {
    int fd = ClientUtil::connect_to_server(argv[1], argv[2]);
    FdGuard guard(fd);

    // prompt the user for credentials and log in to the server
    std::string credentials = ClientUtil::prompt_for_credentials();
    Message login_req(MessageType::LOGIN, ClientMode::DISPLAY, credentials);
    Message login_response = send_request_receive_response(fd, login_req);

    if (login_response.get_type() == MessageType::ERROR) {
      std::cerr << "Error: " << login_response.get_str() << "\n";
      return 1;
    }

    // the only valid response to a LOGIN request is OK
    if (login_response.get_type() != MessageType::OK)
      throw ProtocolError("Unexpected response type");

    std::map<int, std::shared_ptr<Order>> orders;
    refresh_display(orders);

    // main loop 
    while (true) {
      Message msg;
      ClientUtil::receive_message(fd, msg);

      bool should_refresh = true;

      // handle the message and update the orders data structure as needed
      if (msg.get_type() == MessageType::DISP_ORDER) {
        handle_disp_order(msg, orders);
      } else if (msg.get_type() == MessageType::DISP_ITEM_UPDATE) {
        handle_disp_item_update(msg, orders);
      } else if (msg.get_type() == MessageType::DISP_ORDER_UPDATE) {
        handle_disp_order_update(msg, orders);
      } else if (msg.get_type() == MessageType::DISP_HEARTBEAT) {
        should_refresh = false;
      } else {
        throw ProtocolError("unexpected message type in display loop");
      }
      
      // refresh the display after handling the message
      if (should_refresh)
        refresh_display(orders);
    }

    return 0;
  } catch (Exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}
