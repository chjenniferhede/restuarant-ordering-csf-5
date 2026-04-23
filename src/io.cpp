#include <sstream>
#include <iomanip>
#include <cassert>
#include <cctype> // for std::isdigit
#include "csapp.h"
#include "except.h"
#include "io.h"

namespace {

// New helper functions
// given a payload length, return the 4-byte length string
std::string make_length_prefix(size_t payload_len_with_newline) {
  if (payload_len_with_newline > 9999)
    throw IOException("message too long to frame");

  std::ostringstream out;
  out << std::setw(4) << std::setfill('0') << payload_len_with_newline;
  return out.str();
}

// given a 4-byte length prefix, parse it into an int
int parse_length_prefix(const char prefix[4]) {
  for (int i = 0; i < 4; ++i) {
    if (!std::isdigit(static_cast<unsigned char>(prefix[i])))
      throw IOException("invalid length prefix");
  }

  int n = std::stoi(std::string(prefix, 4));
  if (n <= 0)
    throw IOException("invalid framed length");

  return n;
}


} // end of anonymous namespace for helper functions

namespace IO {

void send(const std::string &s, int outfd) {
  std::string framed = make_length_prefix(s.size() + 1) + s + "\n";

  // rio_writen (fd, data, size) is a function to write data to a file descriptor
  // outfd is the socket file descriptor to write to
  // return number of bytes written, or -1 on error
  ssize_t n = rio_writen(outfd, framed.data(), framed.size());
  if (n != ssize_t(framed.size()))
    throw IOException("error sending framed message");
}

void receive(int infd, std::string &s) {
  // infd is the socket file descriptor to read from
  // s is the string to store the received message in
  char size_prefix[4];

  // rio_readn (fd, buffer, size) read data from a file descriptor and put it in the buffer
  // return number of bytes read, or -1 on error
  ssize_t n = rio_readn(infd, size_prefix, sizeof(size_prefix));
  if (n != ssize_t(sizeof(size_prefix)))
    throw IOException("error reading length prefix");

  int framed_len = parse_length_prefix(size_prefix);

  // Create a string of the appropriate length to hold the framed message, and read it in
  std::string body(framed_len, '\0');
  
  // We already consumed the size, so the rest should be body.size
  n = rio_readn(infd, &body[0], body.size());
  if (n != ssize_t(body.size()))
    throw IOException("error reading framed payload");

  // The body should end with a newline, and the rest is the message string
  if (body.back() != '\n')
    throw IOException("missing terminating newline in framed payload");

  // build the result string by taking all but the last character of body
  s.assign(body.data(), body.size() - 1);
}

} // end of IO namespace
