#ifndef GUARD_H
#define GUARD_H

#include <pthread.h>
#include "util.h"

// Mutex guard class.
// Its constructor locks the mutex, and its destructor
// unlocks the mutex. Use a Guard object to make a block
// in the program into a critical section. Guarantees that
// the mutex will be unlocked regardless of how control
// leaves the critical section.
class Guard {
private:
  pthread_mutex_t &m_lock;

  NO_VALUE_SEMANTICS(Guard);

public:
  Guard(pthread_mutex_t &lock)
    : m_lock(lock) {
    pthread_mutex_lock(&m_lock);
  }

  ~Guard() {
    pthread_mutex_unlock(&m_lock);
  }
};

#endif // GUARD_H
