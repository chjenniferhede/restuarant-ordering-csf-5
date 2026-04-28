#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <pthread.h>
#include "util.h"
#include "model.h"
#include "message.h"

// Forward declarations
class Client;

class Server {
private:
  pthread_mutex_t m_mutex;
  std::map<int, std::shared_ptr<Order>> m_orders;
  std::unordered_set<Client *> m_display_clients;
  int m_next_order_id = 1000;

  void broadcast(std::shared_ptr<Message> msg);

  NO_VALUE_SEMANTICS(Server);

public:
  Server();
  ~Server();

  // Create a server socket listening on specified port,
  // and accept connections from clients. This function does
  // not return.
  void server_loop(const char *port);

  void register_display(Client *client);
  void unregister_display(Client *client);

  // Returns the server-assigned order id for the new order.
  int process_new_order(std::shared_ptr<Order> order);

  // Throws SemanticError if the update is invalid.
  void process_item_update(int order_id, int item_id, ItemStatus item_status);
  void process_order_update(int order_id, OrderStatus order_status);
};

#endif // SERVER_H
