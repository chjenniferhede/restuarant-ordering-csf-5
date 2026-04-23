#include "client_util.h"
#include <iostream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"

namespace ClientUtil {

int connect_to_server(const std::string &hostname, const std::string &port) {
	int fd = open_clientfd(hostname.c_str(), port.c_str());
	if (fd < 0)
		throw IOException("could not connect to server");
	return fd;
}

std::string prompt_for_credentials() {
	std::string username;
	std::string password;

	std::cout << "username: " << std::flush;
	std::getline(std::cin, username);

	std::cout << "password: " << std::flush;
	std::getline(std::cin, password);

	return username + "/" + password;
}

void send_message(int fd, const Message &msg) {
	std::string encoded;
	Wire::encode(msg, encoded);
	IO::send(encoded, fd);
}

void receive_message(int fd, Message &msg) {
	std::string encoded;
	IO::receive(fd, encoded);
	Wire::decode(encoded, msg);
}

int parse_positive_int(const std::string &s, const char *field_name) {
	size_t pos = 0;
	int value = -1;

	try {
		value = std::stoi(s, &pos, 10);
	} catch (std::exception &) {
		throw ProtocolError(std::string("invalid integer for ") + field_name);
	}

	if (pos != s.size() || value <= 0)
		throw ProtocolError(std::string("invalid integer for ") + field_name);

	return value;
}

}
