#include <iostream>
#include <cassert>
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
    }

    while (true) {
      // get a message from the client
      std::string client_raw_msg;
      IO::receive(m_fd, client_raw_msg);
      Message client_msg;
      Wire::decode(client_raw_msg, client_msg);

      // if client wants to quit
      if (client_msg.get_type() == MessageType::QUIT) {
        // respond with an OK message
        Message resp(MessageType::OK, std::string("Client disconnected"));
        std::string out;
        Wire::encode(resp, out);
        IO::send(out, m_fd);
        break;
      } else { 

      }
    }

