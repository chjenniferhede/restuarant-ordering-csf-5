#include <cassert>
#include <unistd.h>
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "server.h"
#include "passwd_db.h"
#include "client.h"

Client::Client(int fd, Server *server)
  : m_fd(fd)
  , m_server(server) {
}

Client::~Client() {
  // if m_fd is valid
  if (m_fd >= 0)
    close(m_fd);
}

// Helper to send a message, false on failure
bool Client::send_message(const Message &msg) {
  try {
    std::string encoded;
    Wire::encode(msg, encoded);
    IO::send(encoded, m_fd);
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

// Helper to run the display loop
void Client::display_loop() {
  m_server->register_display(this);

  try {
    while (m_running) {
      auto msg_ptr = m_queue.dequeue();
      if (msg_ptr) {
        // send message, if fail, break
        if (!send_message(*msg_ptr)) break;
      } else {
        // send heartbeat, if fail, break
        if (!send_message(Message(MessageType::DISP_HEARTBEAT))) break;
      }
    }
  // catch all exceptions, do nothing since in all case we just unregister
  } catch (...) {}

  m_server->unregister_display(this);
}

// Helper to run the updater loop
void Client::updater_loop() {
  while (m_running) {
    try {
      Message msg;
      std::string raw;
      IO::receive(m_fd, raw);
      Wire::decode(raw, msg);

      // QUIT
      if (msg.get_type() == MessageType::QUIT) {
        send_message(Message(MessageType::OK, "bye"));
        return;
      // ORDER_NEW
      } else if (msg.get_type() == MessageType::ORDER_NEW) {
        if (!msg.has_order()) {
          send_message(Message(MessageType::ERROR, "missing order"));
          continue;
        }
        int id = m_server->process_new_order(msg.get_order());
        send_message(Message(MessageType::OK, "Created order id " + std::to_string(id)));
      // ITEM_UPDATE
      } else if (msg.get_type() == MessageType::ITEM_UPDATE) {
        if (!msg.has_order_id() || !msg.has_item_id() || !msg.has_item_status()) {
          send_message(Message(MessageType::ERROR, "missing item fields"));
          continue;
        }
        try {
          m_server->process_item_update(msg.get_order_id(), msg.get_item_id(), msg.get_item_status());
          send_message(Message(MessageType::OK, "successful item update"));
        } catch (const SemanticError &e) {
          send_message(Message(MessageType::ERROR, e.what()));
        }
      // ORDER_UPDATE
      } else if (msg.get_type() == MessageType::ORDER_UPDATE) {
        if (!msg.has_order_id() || !msg.has_order_status()) {
          send_message(Message(MessageType::ERROR, "missing order fields"));
          continue;
        }
        try {
          m_server->process_order_update(msg.get_order_id(), msg.get_order_status());
          send_message(Message(MessageType::OK, "successful order update"));
        } catch (const SemanticError &e) {
          send_message(Message(MessageType::ERROR, e.what()));
        }
      } else {
        send_message(Message(MessageType::ERROR, "Updater: unexpected message type"));
      }
    
      // Loop will continue, so next m_running check will end the loop
    } catch (const IOException &) {
      m_running = false;
    } catch (const InvalidMessage &) {
      m_running = false;
    }
  }
}

void Client::chat() {
  try {
    // Get login message
    Message login_msg;
    std::string raw;
    IO::receive(m_fd, raw);
    Wire::decode(raw, login_msg);

    if (login_msg.get_type() != MessageType::LOGIN)
      throw ProtocolError("expected LOGIN");

    if (!login_msg.has_client_mode() || !login_msg.has_str())
      throw ProtocolError("LOGIN missing fields");

    ClientMode mode = login_msg.get_client_mode();
    bool authenticated = PasswordDB::authenticate(login_msg.get_str());

    // If not authenticated, send error and return, end the connection
    if (!authenticated) {
      send_message(Message(MessageType::ERROR, "Invalid credentials"));
      return;
    }

    // Send OK
    send_message(Message(MessageType::OK, "Login successful"));
    m_mode = mode;

    // Enter corresponding loop
    if (m_mode == ClientMode::DISPLAY) {
      display_loop();
    } else if (m_mode == ClientMode::UPDATER) {
      updater_loop();
    } else {
      throw ProtocolError("invalid client mode");
    }
  } catch (const std::exception &) {
    return; // unrecoverable, end connection, trigger ~Client
  }
}
