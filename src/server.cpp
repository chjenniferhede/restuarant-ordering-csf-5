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
  // passed in  
  pthread_mutex_init(&m_mutex, nullptr);
}

Server::~Server() {
  pthread_mutex_destroy(&m_mutex);
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

void Server::broadcast(std::shared_ptr<Message> msg) {
  for (Client *client : m_display_clients)
    client->enqueue(msg->duplicate());
}

void Server::register_display(Client *client) {
  Guard g(m_mutex);
  m_display_clients.insert(client);
  for (auto &pair : m_orders)
    client->enqueue(std::make_shared<Message>(MessageType::DISP_ORDER, pair.second->duplicate()));
}

void Server::unregister_display(Client *client) {
  Guard g(m_mutex);
  m_display_clients.erase(client);
}

int Server::process_new_order(std::shared_ptr<Order> order) {
  Guard g(m_mutex);
  int id = m_next_order_id++;
  order->set_id(id);
  for (int i = 0; i < order->get_num_items(); ++i)
    order->at(i)->set_order_id(id);
  order->set_status(OrderStatus::NEW);
  m_orders[id] = order;
  broadcast(std::make_shared<Message>(MessageType::DISP_ORDER, order));
  return id;
}

void Server::process_item_update(int order_id, int item_id, ItemStatus item_status) {
  Guard g(m_mutex);

  auto oit = m_orders.find(order_id);
  if (oit == m_orders.end())
    throw SemanticError("order not found");

  auto order = oit->second;
  auto item = order->find_item(item_id);
  if (!item)
    throw SemanticError("item not found");

  // Validate strictly forward transition
  ItemStatus cur = item->get_status();
  if (!((cur == ItemStatus::NEW && item_status == ItemStatus::IN_PROGRESS) ||
        (cur == ItemStatus::IN_PROGRESS && item_status == ItemStatus::DONE)))
    throw SemanticError("invalid item status transition");

  item->set_status(item_status);
  broadcast(std::make_shared<Message>(MessageType::DISP_ITEM_UPDATE, order_id, item_id, item_status));

  // Auto-transition order status
  OrderStatus order_status = order->get_status();
  if (item_status == ItemStatus::IN_PROGRESS && order_status == OrderStatus::NEW) {
    order->set_status(OrderStatus::IN_PROGRESS);
    broadcast(std::make_shared<Message>(MessageType::DISP_ORDER_UPDATE, order_id, OrderStatus::IN_PROGRESS));
  } else if (item_status == ItemStatus::DONE && order_status == OrderStatus::IN_PROGRESS) {
    bool all_done = true;
    for (int i = 0; i < order->get_num_items(); ++i)
      if (order->at(i)->get_status() != ItemStatus::DONE) { all_done = false; break; }
    if (all_done) {
      order->set_status(OrderStatus::DONE);
      broadcast(std::make_shared<Message>(MessageType::DISP_ORDER_UPDATE, order_id, OrderStatus::DONE));
    }
  }
}

void Server::process_order_update(int order_id, OrderStatus order_status) {
  Guard g(m_mutex);

  auto oit = m_orders.find(order_id);
  if (oit == m_orders.end())
    throw SemanticError("order not found");

  auto order = oit->second;
  if (order->get_status() != OrderStatus::DONE || order_status != OrderStatus::DELIVERED)
    throw SemanticError("invalid order status transition");

  order->set_status(OrderStatus::DELIVERED);
  broadcast(std::make_shared<Message>(MessageType::DISP_ORDER_UPDATE, order_id, OrderStatus::DELIVERED));
  m_orders.erase(oit);
}
