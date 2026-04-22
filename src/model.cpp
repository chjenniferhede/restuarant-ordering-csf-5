#include <cassert>
#include "model.h"

Item::Item(int order_id, int id, ItemStatus status, const std::string &desc, int qty)
  : m_order_id(order_id)
  , m_id(id)
  , m_status(status)
  , m_desc(desc)
  , m_qty(qty) {
}

Item::~Item() {
}

bool Item::operator==(const Item &rhs) const {
  return get_id() == rhs.get_id() &&
         get_order_id() == rhs.get_order_id() &&
         get_status() == rhs.get_status() &&
         get_desc() == rhs.get_desc() &&
         get_qty() == rhs.get_qty();
}

Order::Order(int id, OrderStatus status)
  : m_id(id)
  , m_status(status) {
}

Order::Order(const Order &other) {
  *this = other;
}

Order::~Order() {
}

Order &Order::operator=(const Order &rhs) {
  if (this != &rhs) {
    m_id = rhs.m_id;
    // Duplicate all of the Items, so that this is
    // a deep copy
    for (auto i = rhs.m_items.begin(); i != rhs.m_items.end(); ++i)
      m_items.push_back((*i)->duplicate());
    m_status = rhs.m_status;
  }
  return *this;
}

bool Order::operator==(const Order &rhs) const {
  if (get_id() != rhs.get_id() ||
      get_status() != rhs.get_status() ||
      get_num_items() != rhs.get_num_items())
    return false;

  // see whether all items are equal
  for (int i = 0; i < get_num_items(); ++i) {
    if (*at(i) != *rhs.at(i))
      return false;
  }

  return true;
}

void Order::add_item(std::shared_ptr<Item> item) {
  m_items.push_back(item);
}

std::shared_ptr<Item> Order::at(int index) const {
  return m_items.at(index);
}

std::shared_ptr<Item> Order::find_item(int item_id) const {
  for (auto i = m_items.begin(); i != m_items.end(); ++i)
    if ((*i)->get_id() == item_id)
      return *i;

  // no such item
  return std::shared_ptr<Item>();
}

// TODO: define additional member functions
