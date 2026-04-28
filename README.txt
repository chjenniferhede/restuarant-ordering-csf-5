
Worked alone

Synchronization report :

Shared data and how we protect it:

- m_orders (map from order id to shared_ptr<Order> inside Server):
   guarded by Server::m_mutex. Whenever we create an order, update an item,
   update an order status, or send a snapshot to a display client, we hold
   this mutex using the Guard RAII helper so those operations don't race.

- m_display_clients (unordered_set of Client* inside Server): also
   guarded by Server::m_mutex. register_display and unregister_display lock
   the mutex before they change the set. When the server broadcasts messages
   it iterates the set while holding the mutex so a client can't be removed
   while we're sending.

- m_next_order_id (int inside Server): incremented while holding
   Server::m_mutex in process_new_order, so order ids don't collide.

- MessageQueue::m_queue (deque of messages inside each MessageQueue):
   guarded by MessageQueue::m_mutex. enqueue locks the queue, pushes the
   message, then calls sem_post to signal availability. dequeue waits up to
   one second with sem_timedwait, then locks and pops the front element; on
   timeout dequeue returns a null pointer and the display loop sends a
   DISP_HEARTBEAT.

Synchronization primitives use:

- pthread_mutex_t for the server mutex and for each MessageQueue's mutex.
   We use the Guard RAII class so locks are released automatically.
- sem_t for the MessageQueue availability semaphore (sem_post /
   sem_timedwait with a 1-second timeout).

Why deadlocks are unlikely:

We don't hold the server mutex and a queue mutex at the same time. The
server releases its mutex before adding messages to a MessageQueue, and
MessageQueue code doesn't call back into the server, so things shouldn't
get stuck waiting on each other in normal operation.
