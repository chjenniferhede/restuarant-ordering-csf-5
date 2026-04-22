#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <memory>

// Client modes
enum class ClientMode {
  INVALID,     // not a valid client mode
  UPDATER,     // updater client
  DISPLAY,     // display client
};

enum class ItemStatus {
  INVALID,       // not a valid Item status
  NEW,           // order exists, kitchen has not started preparing the item
  IN_PROGRESS,   // kitchen has started preparing the item
  DONE,          // kitchen has completed the item
};

class Item {
private:
  int m_order_id;      // unique id of order item belongs to
  int m_id;            // item unique id
  ItemStatus m_status; // item status
  std::string m_desc;  // item description
  int m_qty;           // item quantity

public:
  Item(int order_id, int id, ItemStatus status, const std::string &desc, int qty);
  ~Item();

  // Return a shared_ptr to a deep copy of this Item
  std::shared_ptr<Item> duplicate() const {
    return std::make_shared<Item>(*this);
  }

  // Equality comparison
  bool operator==(const Item &rhs) const;

  // Inequality comparison
  bool operator!=(const Item &rhs) const {
    return !(*this == rhs);
  }

  // Get the item id
  int get_id() const { return m_id; }

  // Get the order id
  int get_order_id() const { return m_order_id; }

  // Set the order id: this is useful in the Server when
  // a new Order needs to be given a "real" order id
  void set_order_id(int order_id) { m_order_id = order_id; }

  // Get the ItemStatus
  ItemStatus get_status() const { return m_status; }

  // Set the ItemStatus
  void set_status(ItemStatus status) { m_status = status; }

  // Get the item description
  std::string get_desc() const { return m_desc; }

  // Get the quantity
  int get_qty() const { return m_qty; }
};

enum class OrderStatus {
  INVALID,      // not a valid Order status
  NEW,          // order has been created
  IN_PROGRESS,  // kitchen has started preparing the order
  DONE,         // kitchen has completed all items in order
  DELIVERED,    // order has been delivered to the customer
};

class Order {
private:
  int m_id;                                   // order unique id
  OrderStatus m_status;                       // order status
  std::vector<std::shared_ptr<Item>> m_items; // items in order

public:
  Order(int id, OrderStatus status);
  Order(const Order &other);
  ~Order();

  // Return a shared_ptr to a deep copy of this Order
  std::shared_ptr<Order> duplicate() const {
    return std::make_shared<Order>(*this);
  }

  // Assignment operator
  Order &operator=(const Order &rhs);

  // Equality comparison
  bool operator==(const Order &rhs) const;

  // Inequality comparison
  bool operator!=(const Order &rhs) const {
    return !(*this == rhs);
  }

  // Get the order id
  int get_id() const { return m_id; }

  // Set the order id: this is useful in the Server when
  // a new Order needs to be given a "real" order id
  void set_id(int id) { m_id = id; }

  OrderStatus get_status() const { return m_status; }
  void set_status(OrderStatus status) { m_status = status; }

  // Add an item to the order: transfers ownership
  // to the Order
  void add_item(std::shared_ptr<Item> item);

  // Get number of items
  int get_num_items() const { return int(m_items.size()); }

  // Get a shared_ptr to the Item at the specified index
  std::shared_ptr<Item> at(int index) const;

  // Get a shared_ptr to the Item with given item id:
  // returns an "empty" shared_ptr if there is no Item
  // with the specified item id
  std::shared_ptr<Item> find_item(int item_id) const;

  // TODO: add other member functions as needed
};


#endif // MODEL_H
