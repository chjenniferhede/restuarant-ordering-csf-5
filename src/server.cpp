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

#include <pthread.h>
#include <unistd.h>

namespace {

  // Helper function: this function will be called when the caller wants to start a new client thread
  void *client_thread_start(void *client) {
    // Create a unique pointer to the client, so it will be auto deleted
    std::unique_ptr<Client> client_ptr(static_cast<Client*>(client));
    // detach the thread so OS will clean it up when it finishes
    pthread_detach(pthread_self());

    // process the client: enter the chat waiting loop
    try {
      client_ptr->chat();
    } catch (const std::exception &e) {
      std::cerr << "Error in client thread: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown error in client thread" << std::endl;
    }

    return nullptr;
  }

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
    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);

    // try to accept a new client connection: accept is from sys/socket.h and take
    // accept(fd, addr, addrlen), return the file descriptor of the accepted connection
    int connfd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (connfd < 0){ 
      if (errno == EINTR) { // eintr 
        continue;
      } else {
        std::cerr << "Error in accept: " << std::strerror(errno) << std::endl;
        continue;
      }
    }

    // use the file descriptor to create a new client
    Client *client = nullptr;
    try {
      client = new Client(connfd, this); // Client constructor takes (int, Server*)
    } catch (const std::exception &e) {
      std::cerr << "Error creating client: " << e.what() << std::endl;
      close(connfd);
      continue;
    }

    // call pthread_create to start a new thread with the client we just created
    pthread_t thread_id; 
    int return_code = pthread_create(&thread_id, nullptr, client_thread_start, client);
    if (return_code != 0) {
      std::cerr << "Error in pthread_create: " << std::strerror(return_code) << std::endl;
      delete client;
      close(connfd); // close explicitly
      continue;
    }
  }
}

// TODO: other member functions
