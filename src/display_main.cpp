// Main program for the display client

#include <iostream>
#include <string>
#include <stdexcept>
#include "csapp.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "util.h"
#include "client_util.h"

namespace {

// This terminal escape sequence should be written to
// std::cout before the display client redisplays the current
// order information.
const char CLEAR_SCREEN[] = "\x1b[2J\x1b[H";

}

// TODO: implement helper functions/clases as needed

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <hostname> <port>\n";
    return 1;
  }

  // TODO: implement
}
