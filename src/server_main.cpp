#include <iostream>
#include <string>
#include <csignal>
#include "csapp.h"
#include "except.h"
#include "server.h"

int main(int argc, char **argv) {
  // Ignore SIGPIPE. This signal would otherwise be raised
  // if the server attempts to write to a socket where the
  // client has terminated. This can happen, for example, if
  // you use control-C to terminate a display client, and
  // the server tries to send a message to it.
  signal(SIGPIPE, SIG_IGN);

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>\n";
    return 1;
  }

  Server server;

  try {
    server.server_loop(argv[1]);
  } catch (IOException &ex) {
    std::cerr << "Error: " << ex.what() << "\n";
  }
}
