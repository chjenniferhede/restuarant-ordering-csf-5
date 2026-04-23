
#include <iostream>
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
// helper functions for handling commands
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


// Handle commands
void handle_order_new(int fd) {
  std::string line;
  std::getline(std::cin, line);
  int num_items = ClientUtil::parse_positive_int(line, "number of items");

  // create a new order object with a temporary order id of 1 and status of NEW
  auto order = std::make_shared<Order>(1, OrderStatus::NEW);

  // for each item, read item id, description, and quantity from stdin and add to order
  for (int i = 0; i < num_items; ++i) {
    std::string item_id_s;
    std::string item_desc;
    std::string item_qty_s;

    std::getline(std::cin, item_id_s);
    std::getline(std::cin, item_desc);
    std::getline(std::cin, item_qty_s);

    int item_id = ClientUtil::parse_positive_int(item_id_s, "item id");
    int item_qty = ClientUtil::parse_positive_int(item_qty_s, "item quantity");

    // create a new item: 
    order->add_item(std::make_shared<Item>(1, item_id, ItemStatus::NEW, item_desc, item_qty));
  }

  // send the request and print the response
  Message request(MessageType::ORDER_NEW, order);
  Message response = send_request_receive_response(fd, request);
  print_result_response(response);
}

void handle_item_update(int fd) {
  std::string order_id_s;
  std::string item_id_s;
  std::string item_status_s;

  // read order id, item id, and item status from stdin
  std::getline(std::cin, order_id_s);
  std::getline(std::cin, item_id_s);
  std::getline(std::cin, item_status_s);

  // parse order id and item id as positive integers
  int order_id = ClientUtil::parse_positive_int(order_id_s, "order id");
  int item_id = ClientUtil::parse_positive_int(item_id_s, "item id");

  ItemStatus item_status = Wire::str_to_item_status(item_status_s);
  if (item_status == ItemStatus::INVALID)
    throw ProtocolError("invalid item status");

  // send the request and print the response
  Message request(MessageType::ITEM_UPDATE, order_id, item_id, item_status);
  Message response = send_request_receive_response(fd, request);
  print_result_response(response);
}

void handle_order_update(int fd) {
  std::string order_id_s;
  std::string order_status_s;

  std::getline(std::cin, order_id_s);
  std::getline(std::cin, order_status_s);

  // parse order id as positive integer
  int order_id = ClientUtil::parse_positive_int(order_id_s, "order id");
  OrderStatus order_status = Wire::str_to_order_status(order_status_s);
  if (order_status == OrderStatus::INVALID)
    throw ProtocolError("invalid order status");

  // send the request and print the response
  Message request(MessageType::ORDER_UPDATE, order_id, order_status);
  Message response = send_request_receive_response(fd, request);
  print_result_response(response);
}

}

// main program for updater client
int main(int argc, char **argv) {

  // Check command line arguments: expect hostname and port
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <hostname> <port>\n";
    return 1;
  }

  try {

    int filedescriptor = ClientUtil::connect_to_server(argv[1], argv[2]);
    FdGuard guard(filedescriptor);

    // Prompt for credentials and log in
    std::string user_typed_credentials = ClientUtil::prompt_for_credentials();

    // create a login message and send it 
    Message login_req(MessageType::LOGIN, ClientMode::UPDATER, user_typed_credentials);
    Message login_response = send_request_receive_response(filedescriptor, login_req);

    // check the response
    if (login_response.get_type() == MessageType::ERROR) {
      std::cerr << "Error: " << login_response.get_str() << "\n";
      return 1;
    }
    if (login_response.get_type() != MessageType::OK)
      throw ProtocolError("Unexpected response type");

    // If we get here, login successful
    // start the loop waiting for comamnds
    while (true) {
      // print prompt
      std::cout << "> " << std::flush;

      // if user types EOF (e.g. ctrl-d), exit the loop and close the connection
      std::string command;
      if (!std::getline(std::cin, command)) // get line returns false if EOF
        break;

      // or if user types "quit", send quit message to server and exit
      if (command == "quit") {
        Message quit_req(MessageType::QUIT, "quit");
        Message response = send_request_receive_response(filedescriptor, quit_req);
        if (response.get_type() != MessageType::OK)
          throw ProtocolError("Unexpected response type");
        return 0;
      }

      // other commands: order_new, item_update, order_update
      if (command == "order_new") {
        handle_order_new(filedescriptor);
      } else if (command == "item_update") {
        handle_item_update(filedescriptor);
      } else if (command == "order_update") {
        handle_order_update(filedescriptor);
      }
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
