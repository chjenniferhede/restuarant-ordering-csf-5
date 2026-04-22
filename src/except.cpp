#include "except.h"

Exception::Exception(const std::string &desc)
  : std::runtime_error(desc) {
}

Exception::Exception(const Exception &other)
  : std::runtime_error(other.what()) {
}

Exception::~Exception() {
}

InvalidMessage::InvalidMessage(const std::string &desc)
  : Exception(desc) {
}

InvalidMessage::InvalidMessage(const InvalidMessage &other)
  : Exception(other) {
}

InvalidMessage::~InvalidMessage() {
}

IOException::IOException(const std::string &desc)
  : Exception(desc) {
}

IOException::IOException(const IOException &other)
  : Exception(other) {
}

IOException::~IOException() {
}

ProtocolError::ProtocolError(const std::string &desc)
  : Exception(desc) {
}

ProtocolError::ProtocolError(const ProtocolError &other)
  : Exception(other) {
}

ProtocolError::~ProtocolError() {
}

SemanticError::SemanticError(const std::string &desc)
  : Exception(desc) {
}

SemanticError::SemanticError(const SemanticError &other)
  : Exception(other) {
}

SemanticError::~SemanticError() {
}
