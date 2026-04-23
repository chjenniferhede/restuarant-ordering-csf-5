// Main program for the updater client

#include <iostream>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include "csapp.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "util.h"
#include "client_util.h"

namespace {

class FdGuard {
private:
  int m_fd;

public:
  explicit FdGuard(int fd)
    : m_fd(fd) {
  }

  ~FdGuard() {
    if (m_fd >= 0)
      close(m_fd);
  }
};

void print_result_response(const Message &response) {
  if (response.get_type() == MessageType::OK) {
    std::cout << "Success: " << response.get_str() << "\n";
  } else if (response.get_type() == MessageType::ERROR) {
    std::cout << "Failure: " << response.get_str() << "\n";
  } else {
    throw ProtocolError("Unexpected response type");
  }
}

Message send_request_receive_response(int fd, const Message &request) {
  ClientUtil::send_message(fd, request);

  Message response;
  ClientUtil::receive_message(fd, response);
  return response;
}

void handle_order_new(int fd) {
  std::string line;
  std::getline(std::cin, line);
  int num_items = ClientUtil::parse_positive_int(line, "number of items");

  auto order = std::make_shared<Order>(1, OrderStatus::NEW);

  for (int i = 0; i < num_items; ++i) {
    std::string item_id_s;
    std::string item_desc;
    std::string item_qty_s;

    std::getline(std::cin, item_id_s);
    std::getline(std::cin, item_desc);
    std::getline(std::cin, item_qty_s);

    int item_id = ClientUtil::parse_positive_int(item_id_s, "item id");
    int item_qty = ClientUtil::parse_positive_int(item_qty_s, "item quantity");

    order->add_item(std::make_shared<Item>(1, item_id, ItemStatus::NEW, item_desc, item_qty));
  }

  Message request(MessageType::ORDER_NEW, order);
  Message response = send_request_receive_response(fd, request);
  print_result_response(response);
}

void handle_item_update(int fd) {
  std::string order_id_s;
  std::string item_id_s;
  std::string item_status_s;

  std::getline(std::cin, order_id_s);
  std::getline(std::cin, item_id_s);
  std::getline(std::cin, item_status_s);

  int order_id = ClientUtil::parse_positive_int(order_id_s, "order id");
  int item_id = ClientUtil::parse_positive_int(item_id_s, "item id");

  ItemStatus item_status = Wire::str_to_item_status(item_status_s);
  if (item_status == ItemStatus::INVALID)
    throw ProtocolError("invalid item status");

  Message request(MessageType::ITEM_UPDATE, order_id, item_id, item_status);
  Message response = send_request_receive_response(fd, request);
  print_result_response(response);
}

void handle_order_update(int fd) {
  std::string order_id_s;
  std::string order_status_s;

  std::getline(std::cin, order_id_s);
  std::getline(std::cin, order_status_s);

  int order_id = ClientUtil::parse_positive_int(order_id_s, "order id");
  OrderStatus order_status = Wire::str_to_order_status(order_status_s);
  if (order_status == OrderStatus::INVALID)
    throw ProtocolError("invalid order status");

  Message request(MessageType::ORDER_UPDATE, order_id, order_status);
  Message response = send_request_receive_response(fd, request);
  print_result_response(response);
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

    std::string credentials = ClientUtil::prompt_for_credentials();
    Message login_req(MessageType::LOGIN, ClientMode::UPDATER, credentials);
    Message login_response = send_request_receive_response(fd, login_req);

    if (login_response.get_type() == MessageType::ERROR) {
      std::cerr << "Error: " << login_response.get_str() << "\n";
      return 1;
    }
    if (login_response.get_type() != MessageType::OK)
      throw ProtocolError("Unexpected response type");

    while (true) {
      std::cout << "> " << std::flush;

      std::string command;
      if (!std::getline(std::cin, command))
        break;

      if (command == "quit") {
        Message quit_req(MessageType::QUIT, "quit");
        Message response = send_request_receive_response(fd, quit_req);
        if (response.get_type() != MessageType::OK)
          throw ProtocolError("Unexpected response type");
        return 0;
      }

      if (command == "order_new") {
        handle_order_new(fd);
      } else if (command == "item_update") {
        handle_item_update(fd);
      } else if (command == "order_update") {
        handle_order_update(fd);
      }
    }

    return 0;
  } catch (Exception &ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  } catch (std::exception &ex) {
    std::cerr << "Error: " << ex.what() << "\n";
    return 1;
  }
}
