#ifndef FD_GUARD_H
#define FD_GUARD_H

#include <unistd.h>
#include "util.h"

// File descriptor guard class.
// Its constructor stores a file descriptor, and its destructor
// closes the file descriptor if it is valid.
class FdGuard {
private:
  int m_fd;

  NO_VALUE_SEMANTICS(FdGuard);

public:
  explicit FdGuard(int fd)
    : m_fd(fd) {
  }

  ~FdGuard() {
    if (m_fd >= 0)
      close(m_fd);
  }
};

#endif // FD_GUARD_H
