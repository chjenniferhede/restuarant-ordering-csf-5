#include <iostream>
#include <memory>
#include <cstring>
#include <cerrno>
#include "csapp.h"
#include "except.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "client.h"
#include "guard.h"
#include "server.h"

#include <pthread.h>
#include <unistd.h>

namespace {

void *client_thread_start(void *client) {
  std::unique_ptr<Client> client_ptr(static_cast<Client*>(client));
  pthread_detach(pthread_self());
  client_ptr->chat();
  return nullptr;
}

} // anonymous namespace

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
    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);

    int connfd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (connfd < 0) {
      if (errno != EINTR)
        std::cerr << "accept error: " << std::strerror(errno) << "\n";
      continue;
    }

    Client *client = nullptr;
    try {
      client = new Client(connfd, this);
    } catch (const std::exception &e) {
      std::cerr << "Error creating client: " << e.what() << "\n";
      close(connfd);
      continue;
    }

    pthread_t thread_id;
    int rc = pthread_create(&thread_id, nullptr, client_thread_start, client);
    if (rc != 0) {
      std::cerr << "pthread_create error: " << std::strerror(rc) << "\n";
      delete client;
      continue;
    }
  }
}

// TODO: other member functions
