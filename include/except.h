#ifndef EXCEPT_H
#define EXCEPT_H

#include <stdexcept>

// Base exception type
class Exception : public std::runtime_error {
public:
  Exception(const std::string &desc);
  Exception(const Exception &other);
  ~Exception();
};

// Exception to indicate that a received message was invalid
class InvalidMessage : public Exception {
public:
  InvalidMessage(const std::string &desc);
  InvalidMessage(const InvalidMessage &other);
  ~InvalidMessage();
};

// Exception to indicate that an I/O error occurred
// (e.g., reading from or writing to a socket file descriptor
// failed)
class IOException : public Exception {
public:
  IOException(const std::string &desc);
  IOException(const IOException &other);
  ~IOException();
};

// Exception to indicate that the remote peer did not follow
// the protocol correctly
class ProtocolError : public Exception {
public:
  ProtocolError(const std::string &desc);
  ProtocolError(const ProtocolError &other);
  ~ProtocolError();
};

// Exception to indicate a recoverable semantic error.
// For example, the client sent an invalid order id or
// item id. This type of error is not considered a protocol
// error, but it does mean that the server will send back
// an ERROR response.
class SemanticError : public Exception {
public:
  SemanticError(const std::string &desc);
  SemanticError(const SemanticError &other);
  ~SemanticError();
};

#endif // EXCEPT_H
