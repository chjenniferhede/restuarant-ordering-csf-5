#ifndef IO_H
#define IO_H

#include <string>

namespace IO {

// Write one string to given output file descriptor.
// Throws IOException if an error occurs.
void send(const std::string &s, int outfd);

// Read one string from given input file descriptor.
// Throws IOException if an error occurs.
void receive(int infd, std::string &s);

}

#endif // IO_H
