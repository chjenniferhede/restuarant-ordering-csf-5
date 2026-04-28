#include <ctime>
#include "message_queue.h"

MessageQueue::MessageQueue() {
  pthread_mutex_init(&m_mutex, nullptr);
  sem_init(&m_avail, 0, 0);
}

MessageQueue::~MessageQueue() {
  pthread_mutex_destroy(&m_mutex);
  sem_destroy(&m_avail);
}

void MessageQueue::enqueue(std::shared_ptr<Message> msg) {
  pthread_mutex_lock(&m_mutex);
  m_queue.push_back(msg);
  pthread_mutex_unlock(&m_mutex);
  sem_post(&m_avail);
}

std::shared_ptr<Message> MessageQueue::dequeue() {
  std::timespec ts;
  std::timespec_get(&ts, TIME_UTC);
  ts.tv_sec += 1;

  if (sem_timedwait(&m_avail, &ts) != 0)
    return std::shared_ptr<Message>();

  pthread_mutex_lock(&m_mutex);
  auto msg = m_queue.front();
  m_queue.pop_front();
  pthread_mutex_unlock(&m_mutex);
  return msg;
}
