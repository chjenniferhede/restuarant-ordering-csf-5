#include <iostream>
#include <cassert>
#include <unistd.h>

#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "server.h"
#include "passwd_db.h"
#include "client.h"
#include "message.h"

Client::Client(int fd, Server *server)
  : m_fd(fd)
  , m_server(server) {
  // TODO: do any necessary initialization
}

Client::~Client() {
  // TODO: do any necessary cleanup
}

bool Client::send_message(const Message &msg) {
  std::string out;
  Wire::encode(msg, out); // put message into str
  try {
    IO::send(out, m_fd);
    return true; 
  } catch (const std::exception &e) {
    std::cerr << "Error sending data to client: " << e.what() << std::endl;
    return false; // indicate that sending failed
  }
}

void Client::display_loop() { 
  m_server->register_display(this);
  
  while (m_running) { 
    try { 
      // Dequeue, wait for a message for 1 second
      auto msg_ptr = m_queue.dequeue();
      if (msg_ptr) { 
        if (!send_message(*msg_ptr)) { m_running = false; break; }

      // If we didn't get a message, check heartbeat
      } else { 
        Message heartbeat_msg(MessageType::DISP_HEARTBEAT);
        if (!send_message(heartbeat_msg)) { m_running = false; break; }
      }

    // if any above step failed, we assume client disconnected
    } catch (const std::exception &e) {
      std::cerr << "Error sending data to client: " << e.what() << std::endl;
      m_running = false;
      break;
    } 
  } // loop end

  m_server->unregister_display(this);
  close(m_fd);
} 

void Client::updater_loop() {
  while (m_running) { 
    try { 
      // receive a message from the client
      std::string raw_msg;
      IO::receive(m_fd, raw_msg);

      // decode the message
      Message msg; 
      Wire::decode(raw_msg, msg);

      // Process quit 
      if (msg.get_type() == MessageType::QUIT) {
        Message resp(MessageType::OK, std::string("Client quitting"));
        send_message(resp);
        m_running = false;
        break;
      }
      // Process new order
      else if (msg.get_type() == MessageType::ORDER_NEW) { 
        if (!msg.has_order()) {
          Message resp(MessageType::ERROR, std::string("New order message missing order"));
          send_message(resp);
          continue;
        } else { 
          int id = m_server->process_new_order(msg.get_order());
          Message resp(MessageType::OK, std::string("Created new order ") + std::to_string(id));
          resp.set_order_id(id);
          send_message(resp);
        }
      }
      // Process item update
      else if (msg.get_type() == MessageType::ITEM_UPDATE) {
        if (!msg.has_order_id() || !msg.has_item_id() || !msg.has_item_status()) {
          Message resp(MessageType::ERROR, std::string("Missing fields in item update message"));
          send_message(resp);
          continue;
        } else {
          bool success = m_server->process_item_update(msg.get_order_id(), 
                                                       msg.get_item_id(), 
                                                       msg.get_item_status());
          Message resp(success ? MessageType::OK : MessageType::ERROR, 
                       success ? std::string("Item update successful") : std::string("Item update failed"));
          send_message(resp);
        }
      }
      // Process order update
      else if (msg.get_type() == MessageType::ORDER_UPDATE) {
        if (!msg.has_order_id() || !msg.has_order_status()) {
          Message resp(MessageType::ERROR, std::string("Missing fields in order update message"));
          send_message(resp);
          continue;
        } else {
          bool success = m_server->process_order_update(msg.get_order_id(), msg.get_order_status());
          Message resp(success ? MessageType::OK : MessageType::ERROR, 
                       success ? std::string("Order update successful") : std::string("Order update failed"));
          send_message(resp);
        }
      }
      // Process unknown message type
      else {
        Message resp(MessageType::ERROR, std::string("Unknown message type"));
        send_message(resp);
        continue;
      }
    } catch (const std::exception &e) {
      std::cerr << "Error processing message from client: " << e.what() << std::endl;
      m_running = false;
      break;
    }
  } // loop end

  close(m_fd);
}
    



void Client::chat() {
  try { 

    // receive a message from the client
    std::string login_raw_msg;
    IO::receive(m_fd, login_raw_msg);

    // decode the message
    Message login_msg; 
    Wire::decode(login_raw_msg, login_msg);

    // check the message type, first must be LOGIN
    if (login_msg.get_type() != MessageType::LOGIN) {
      throw ProtocolError("Expected LOGIN message");
    }

    if (!login_msg.has_client_mode() || !login_msg.has_str()) {
      throw ProtocolError("LOGIN need client_mode and str");
    }

    // get the client mode and credential from the message
    ClientMode mode = login_msg.get_client_mode();
    const std::string &credential = login_msg.get_str();

    // authenticate the client
    bool authenticated = PasswordDB::authenticate(credential);

    // Respond with an error message if authentication fails
    if (!authenticated) { 
      Message resp(MessageType::ERROR, std::string("Invalid credentials"));
      std::string out; 
      Wire::encode(resp, out);
      IO::send(out, m_fd);
      return;
      // return early
    }

    // we know that it success 
    Message resp(MessageType::OK, std::string("Login successful"));
    std::string out;
    Wire::encode(resp, out);
    IO::send(out, m_fd);

    // we can now store client info
    m_mode = mode;

    // --- DISPLAY --- //
    if (m_mode == ClientMode::DISPLAY) {
      display_loop();
      return;
    } else if (m_mode == ClientMode::UPDATER) {
      updater_loop();
      return;
    } else { 
      throw ProtocolError("Invalid client mode");
    }
  } catch (const std::exception &e) {
    std::cerr << "Error during client chat: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown error during client chat" << std::endl;
  }

  if (m_mode == ClientMode::DISPLAY) {
    m_server->unregister_display(this);
  }
  close(m_fd);

}
