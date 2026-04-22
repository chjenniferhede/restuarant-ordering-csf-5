#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <unordered_set>
#include <pthread.h>
#include "util.h"
#include "model.h"

// Forward declarations
class Client;
class Message;

class Server {
private:
  // Private data
  // TODO: add fields

  // no value semantics
  NO_VALUE_SEMANTICS(Server);

public:
  Server();
  ~Server();

  // Create a server socket listening on specified port,
  // and accept connections from clients. This function does
  // not return.
  void server_loop(const char *port);

  // TODO: additional public member functions

private:
  // TODO: private member functions
};

#endif // SERVER_H
