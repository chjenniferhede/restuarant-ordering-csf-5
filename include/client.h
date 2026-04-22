#ifndef CLIENT_H
#define CLIENT_H

#include "util.h"
#include "model.h"
#include "message_queue.h"

class Server;
class Message;

// A Client object is responsible for communicating with one
// remote client. Its job is to receive requests from the remote
// client, process them (possibly invoking methods of the Server
// object), and send back a response for each request.
// In the case that the remote client logs in as a DISPLAY client,
// then the Client object should send DISP_ORDER, DISP_ITEM_UPDATE,
// and DISP_ORDER_UPDATE messages to communicate order and item
// information.
class Client {
private:
  int m_fd; // socket file descriptor for communicating with client
  Server *m_server; // pointer to Server instance
  // TODO: add additional fields if needed

  NO_VALUE_SEMANTICS(Client);

public:
  // Constructor.
  // Parameters:
  //   fd - file descriptor of the TCP socket to be used to communicate
  //        with the remote client
  //   server - pointer to the single Server object which maintains
  //            the data (i.e., the active Order objects and their
  //            Items)
  Client(int fd, Server *server);
  ~Client();

  // Communicate with the remote client.
  // Should be executed in a new thread (i.e., not the server
  // loop thread.) May throw an exception (e.g., IOException
  // if there is an error sending or receiving data over the
  // client socket), so caller should ensure that the thread
  // is terminated nicely and resources are cleaned up.
  void chat();

  // TODO: more public member functions

private:
  // TODO: private member functions
};

#endif // CLIENT_H
