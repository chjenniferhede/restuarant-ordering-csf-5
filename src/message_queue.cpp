#include <iostream>
#include <ctime>
#include "guard.h"
#include "message_queue.h"

MessageQueue::MessageQueue() {
  // TODO: initialization
}

MessageQueue::~MessageQueue() {
  // TODO: cleanup
}

void MessageQueue::enqueue(std::shared_ptr<Message> msg) {
  // TODO: implement
}

std::shared_ptr<Message> MessageQueue::dequeue() {
  // TODO: implement
}
