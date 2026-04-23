#ifndef CLIENT_UTIL_H
#define CLIENT_UTIL_H

#include <string>

class Message;

// This is a way that you can avoid code duplication between
// the updater and display clients

namespace ClientUtil {

// Connect to server and return socket fd.
// Throws IOException if connection fails.
int connect_to_server(const std::string &hostname, const std::string &port);

// Prompt user for credentials and return "username/password".
std::string prompt_for_credentials();

// Send and receive protocol messages.
void send_message(int fd, const Message &msg);
void receive_message(int fd, Message &msg);

// Parse a positive base-10 integer from text.
int parse_positive_int(const std::string &s, const char *field_name);

}

#endif // CLIENT_UTIL_H
