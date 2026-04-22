#include <iostream>
#include <cassert>
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
  // TODO: do any necessary initialization
}

Client::~Client() {
  // TODO: do any necessary cleanup
}

void Client::chat() {
  // TODO: implement
}

// TODO: implement other member functions

