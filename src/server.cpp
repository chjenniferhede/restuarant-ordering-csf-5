#include <iostream>
#include <memory>
#include <cstring> // for strerror()
#include <cerrno>  // for errno
#include "csapp.h"
#include "except.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "client.h"
#include "guard.h"
#include "server.h"

namespace {

// TODO: you can define standalone helper functions here

}

Server::Server() {
  // TODO: initialization
}

Server::~Server() {
}

void Server::server_loop(const char *port) {
  int server_fd = open_listenfd(port);
  if (server_fd < 0)
    throw IOException(std::string("open_listenfd failed: ") + std::strerror(errno));

  while (true) {
    // TODO: accept connections from clients
  }
}

// TODO: other member functions

