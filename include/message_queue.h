#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include <deque>
#include <memory>
#include "util.h"

class Message;

// A queue of Messages.
class MessageQueue {
private:
  std::deque<std::shared_ptr<Message>> m_queue;
  // TODO: synchronization

  // No value semantics
  NO_VALUE_SEMANTICS(MessageQueue);

public:
  MessageQueue();
  ~MessageQueue();

  // Add a Message to the queue.
  void enqueue(std::shared_ptr<Message> msg);

  // Attempt to dequeue a Message.
  // This function should wait at most 1 second for the queue
  // to be non-empty. If a Message is successfully dequeued,
  // the returned shared_ptr will have a non-null stored pointer.
  // If the queue was empty and no Message arrived in 1 second,
  // the returned shared_ptr will have a null stored pointer.
  std::shared_ptr<Message> dequeue();
};

#endif // MESSAGE_QUEUE_H
