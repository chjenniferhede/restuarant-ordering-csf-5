#include "message.h"
#include "wire.h"
#include "except.h"
#include "tctest.h"

struct TestObjs {
  TestObjs();

  std::shared_ptr<Order> order; 
  std::shared_ptr<Order> order_no_items;

  Message login_msg_2, login_msg_3,
          quit_msg, ok_msg, error_msg,
          item_update_msg_1, item_update_msg_2, item_update_msg_3,
          order_new_msg, disp_order_msg,
          order_update_msg_1,
          order_update_msg_2,
          order_update_msg_3,
          disp_item_update_msg_1,
          disp_item_update_msg_2,
          disp_order_update_msg_1,
          disp_order_update_msg_2,
          disp_order_update_msg_3,
          disp_heartbeat_msg,
          order_new_no_items_msg;
};

TestObjs *setup();
void cleanup(TestObjs *objs);

// Test functions
void test_encode_login_2(TestObjs *objs);
void test_encode_login_3(TestObjs *objs);
void test_encode_quit(TestObjs *objs);
void test_encode_ok(TestObjs *objs);
void test_encode_error(TestObjs *objs);
void test_encode_item_update(TestObjs *objs);
void test_encode_order_new(TestObjs *objs);
void test_encode_disp_order(TestObjs *objs);
void test_encode_order_update(TestObjs *objs);
void test_encode_disp_item_update(TestObjs *objs);
void test_encode_disp_order_update(TestObjs *objs);
void test_encode_disp_heartbeat(TestObjs *objs);
void test_encode_order_new_no_items(TestObjs *objs);
void test_decode_login_2(TestObjs *objs);
void test_decode_login_3(TestObjs *objs);
void test_decode_quit(TestObjs *objs);
void test_decode_ok(TestObjs *objs);
void test_decode_error(TestObjs *objs);
void test_decode_item_update(TestObjs *objs);
void test_decode_order_new(TestObjs *objs);
void test_decode_disp_order(TestObjs *objs);
void test_decode_order_update(TestObjs *objs);
void test_decode_disp_item_update(TestObjs *objs);
void test_decode_disp_order_update(TestObjs *objs);
void test_decode_disp_heartbeat(TestObjs *objs);
void test_decode_invalid(TestObjs *objs);
void test_decode_invalid_tricky(TestObjs *objs);
void test_decode_order_new_no_items(TestObjs *objs);

int main(int argc, char **argv) {
  if (argc > 1)
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST(test_encode_login_2);
  TEST(test_encode_login_3);
  TEST(test_encode_quit);
  TEST(test_encode_ok);
  TEST(test_encode_error);
  TEST(test_encode_item_update);
  TEST(test_encode_order_new);
  TEST(test_encode_disp_order);
  TEST(test_encode_order_update);
  TEST(test_encode_disp_item_update);
  TEST(test_encode_disp_order_update);
  TEST(test_encode_disp_heartbeat);
  TEST(test_encode_order_new_no_items);
  TEST(test_decode_login_2);
  TEST(test_decode_login_3);
  TEST(test_decode_quit);
  TEST(test_decode_ok);
  TEST(test_decode_error);
  TEST(test_decode_item_update);
  TEST(test_decode_order_new);
  TEST(test_decode_disp_order);
  TEST(test_decode_order_update);
  TEST(test_decode_disp_item_update);
  TEST(test_decode_disp_order_update);
  TEST(test_decode_disp_heartbeat);
  TEST(test_decode_invalid);
  TEST(test_decode_invalid_tricky);
  TEST(test_decode_order_new_no_items);

  TEST_FINI();
}

// Constructor for TestObjs
TestObjs::TestObjs()
  : login_msg_2(MessageType::LOGIN, ClientMode::UPDATER, "alice/foobar")
  , login_msg_3(MessageType::LOGIN, ClientMode::DISPLAY, "alice/foobar")
  , quit_msg(MessageType::QUIT, "quit")
  , ok_msg(MessageType::OK, "ok")
  , error_msg(MessageType::ERROR, "invalid message")
  , item_update_msg_1(MessageType::ITEM_UPDATE, 42, 109, ItemStatus::NEW)
  , item_update_msg_2(MessageType::ITEM_UPDATE, 17, 55, ItemStatus::IN_PROGRESS)
  , item_update_msg_3(MessageType::ITEM_UPDATE, 12, 4, ItemStatus::DONE)
  , order_update_msg_1(MessageType::ORDER_UPDATE, 43, OrderStatus::IN_PROGRESS)
  , order_update_msg_2(MessageType::ORDER_UPDATE, 43, OrderStatus::DONE)
  , order_update_msg_3(MessageType::ORDER_UPDATE, 43, OrderStatus::DELIVERED)
  , disp_item_update_msg_1(MessageType::DISP_ITEM_UPDATE, 99, 105, ItemStatus::IN_PROGRESS)
  , disp_item_update_msg_2(MessageType::DISP_ITEM_UPDATE, 99, 105, ItemStatus::DONE)
  , disp_order_update_msg_1(MessageType::DISP_ORDER_UPDATE, 83, OrderStatus::IN_PROGRESS)
  , disp_order_update_msg_2(MessageType::DISP_ORDER_UPDATE, 83, OrderStatus::DONE)
  , disp_order_update_msg_3(MessageType::DISP_ORDER_UPDATE, 83, OrderStatus::DELIVERED)
  , disp_heartbeat_msg(MessageType::DISP_HEARTBEAT)
{
  order = std::make_shared<Order>(121, OrderStatus::NEW);
  order->add_item(std::make_shared<Item>(121, 45, ItemStatus::NEW, "Dal Makhani", 1));
  order->add_item(std::make_shared<Item>(121, 67, ItemStatus::NEW, "Samosa", 2));

  // It's invalid to have an Order with no items
  // (should be reported when encoding and decoding)
  order_no_items = std::make_shared<Order>(473, OrderStatus::NEW);

  order_new_msg = Message(MessageType::ORDER_NEW, order);
  disp_order_msg = Message(MessageType::DISP_ORDER, order);
  order_new_no_items_msg = Message(MessageType::ORDER_NEW, order_no_items);
}

TestObjs *setup() {
  return new TestObjs();
}

void cleanup(TestObjs *objs) {
  delete objs;
}

#define TEST_ENCODE(msg_obj, expected_str) do { \
  std::string s; \
  Wire::encode((msg_obj), s); \
  ASSERT(s == (expected_str)); \
} while (0)

void test_encode_login_2(TestObjs *objs) {
  TEST_ENCODE(objs->login_msg_2, "LOGIN|UPDATER|alice/foobar");
}

void test_encode_login_3(TestObjs *objs) {
  TEST_ENCODE(objs->login_msg_3, "LOGIN|DISPLAY|alice/foobar");
}

void test_encode_quit(TestObjs *objs) {
  TEST_ENCODE(objs->quit_msg, "QUIT|quit");
}

void test_encode_ok(TestObjs *objs) {
  TEST_ENCODE(objs->ok_msg, "OK|ok");
}

void test_encode_error(TestObjs *objs) {
  TEST_ENCODE(objs->error_msg, "ERROR|invalid message");
}

void test_encode_item_update(TestObjs *objs) {
  TEST_ENCODE(objs->item_update_msg_1, "ITEM_UPDATE|42|109|NEW");
  TEST_ENCODE(objs->item_update_msg_2, "ITEM_UPDATE|17|55|IN_PROGRESS");
  TEST_ENCODE(objs->item_update_msg_3, "ITEM_UPDATE|12|4|DONE");
}

void test_encode_order_new(TestObjs *objs) {
  TEST_ENCODE(objs->order_new_msg, "ORDER_NEW|121,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
}

void test_encode_disp_order(TestObjs *objs) {
  TEST_ENCODE(objs->disp_order_msg, "DISP_ORDER|121,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
}

void test_encode_order_update(TestObjs *objs) {
  TEST_ENCODE(objs->order_update_msg_1, "ORDER_UPDATE|43|IN_PROGRESS");
  TEST_ENCODE(objs->order_update_msg_2, "ORDER_UPDATE|43|DONE");
  TEST_ENCODE(objs->order_update_msg_3, "ORDER_UPDATE|43|DELIVERED");
}

void test_encode_disp_item_update(TestObjs *objs) {
  TEST_ENCODE(objs->disp_item_update_msg_1, "DISP_ITEM_UPDATE|99|105|IN_PROGRESS");
  TEST_ENCODE(objs->disp_item_update_msg_2, "DISP_ITEM_UPDATE|99|105|DONE");
}

void test_encode_disp_order_update(TestObjs *objs) {
  TEST_ENCODE(objs->disp_order_update_msg_1, "DISP_ORDER_UPDATE|83|IN_PROGRESS");
  TEST_ENCODE(objs->disp_order_update_msg_2, "DISP_ORDER_UPDATE|83|DONE");
  TEST_ENCODE(objs->disp_order_update_msg_3, "DISP_ORDER_UPDATE|83|DELIVERED");
}

void test_encode_disp_heartbeat(TestObjs *objs) {
  TEST_ENCODE(objs->disp_heartbeat_msg, "DISP_HEARTBEAT");
}

void test_encode_order_new_no_items(TestObjs *objs) {
  try {
    std::string s;
    Wire::encode(objs->order_new_no_items_msg, s);
    FAIL("should throw InvalidMessage encoding Order with no Items");
  } catch (InvalidMessage &ex) {
    // good
  }
}

#define TEST_DECODE(expected_msg_obj, encoded_str) do { \
  Message msg; \
  Wire::decode((encoded_str), msg); \
  ASSERT(msg == (expected_msg_obj)); \
} while (0)

void test_decode_login_2(TestObjs *objs) {
  TEST_DECODE(objs->login_msg_2, "LOGIN|UPDATER|alice/foobar");
}

void test_decode_login_3(TestObjs *objs) {
  TEST_DECODE(objs->login_msg_3, "LOGIN|DISPLAY|alice/foobar");
}

void test_decode_quit(TestObjs *objs) {
  TEST_DECODE(objs->quit_msg, "QUIT|quit");
}

void test_decode_ok(TestObjs *objs) {
  TEST_DECODE(objs->ok_msg, "OK|ok");
}

void test_decode_error(TestObjs *objs) {
  TEST_DECODE(objs->error_msg, "ERROR|invalid message");
}

void test_decode_item_update(TestObjs *objs) {
  TEST_DECODE(objs->item_update_msg_1, "ITEM_UPDATE|42|109|NEW");
  TEST_DECODE(objs->item_update_msg_2, "ITEM_UPDATE|17|55|IN_PROGRESS");
  TEST_DECODE(objs->item_update_msg_3, "ITEM_UPDATE|12|4|DONE");
}

void test_decode_order_new(TestObjs *objs) {
  TEST_DECODE(objs->order_new_msg, "ORDER_NEW|121,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
}

void test_decode_disp_order(TestObjs *objs) {
  TEST_DECODE(objs->disp_order_msg, "DISP_ORDER|121,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
}

void test_decode_order_update(TestObjs *objs) {
  TEST_DECODE(objs->order_update_msg_1, "ORDER_UPDATE|43|IN_PROGRESS");
  TEST_DECODE(objs->order_update_msg_2, "ORDER_UPDATE|43|DONE");
  TEST_DECODE(objs->order_update_msg_3, "ORDER_UPDATE|43|DELIVERED");
}

void test_decode_disp_item_update(TestObjs *objs) {
  TEST_DECODE(objs->disp_item_update_msg_1, "DISP_ITEM_UPDATE|99|105|IN_PROGRESS");
  TEST_DECODE(objs->disp_item_update_msg_2, "DISP_ITEM_UPDATE|99|105|DONE");
}

void test_decode_disp_order_update(TestObjs *objs) {
  TEST_DECODE(objs->disp_order_update_msg_1, "DISP_ORDER_UPDATE|83|IN_PROGRESS");
  TEST_DECODE(objs->disp_order_update_msg_2, "DISP_ORDER_UPDATE|83|DONE");
  TEST_DECODE(objs->disp_order_update_msg_3, "DISP_ORDER_UPDATE|83|DELIVERED");
}

void test_decode_disp_heartbeat(TestObjs *objs) {
  TEST_DECODE(objs->disp_heartbeat_msg, "DISP_HEARTBEAT");
}

void TEST_DECODE_INVALID(const std::string &s) {
  Message msg;
  try {
    Wire::decode(s, msg);
    FAIL("Wire::decode failed to throw an exception for invalid encoded message");
  } catch (InvalidMessage &ex) {
    /* good */
  }
}

void test_decode_invalid(TestObjs *objs) {
  // There are many ways an encoded message can fail to be valid,
  // so these tests are not exhaustive. You might want to add
  // tests for additional examples of invalid encoded messages.

  TEST_DECODE_INVALID("FOOBAR|blat");
  TEST_DECODE_INVALID("INVALID|blah");
  TEST_DECODE_INVALID("LOGIN");
  TEST_DECODE_INVALID("LOGIN|alice/foobar|123");
  TEST_DECODE_INVALID("OK|chunky bacon|101");
  TEST_DECODE_INVALID("ERROR|blah blah|99|100");
  TEST_DECODE_INVALID("QUIT|kthxbye|yo");
  TEST_DECODE_INVALID("ITEM_UPDATE|101|202|FOOBAR"); // bad ItemStatus
  TEST_DECODE_INVALID("ITEM_UPDATE|0|14|IN_PROGRESS"); // bad order id
  TEST_DECODE_INVALID("ITEM_UPDATE|11|-3|DONE"); // bad item id

  // invalid order id
  TEST_DECODE_INVALID("ORDER_NEW|-2,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");

  // invalid order status
  TEST_DECODE_INVALID("ORDER_NEW|121,FOOBAR,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");

  // invalid item number
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:0:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");

  // invalid item status
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:45:FOOBAR:Dal Makhani:1;121:67:NEW:Samosa:2");
 
  // invalid quantity in item
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:45:NEW:Dal Makhani:0;121:67:NEW:Samosa:2");

  // invalid number of fields in item
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2:67");

  // bad order id
  TEST_DECODE_INVALID("ITEM_UPDATE|0|55|IN_PROGRESS");

  // bad item id
  TEST_DECODE_INVALID("ITEM_UPDATE|17|-42|IN_PROGRESS");

  // bad item status
  TEST_DECODE_INVALID("ITEM_UPDATE|17|55|SPLUNGE");

  // bad order id
  TEST_DECODE_INVALID("ORDER_UPDATE|0|DELIVERED");

  // bad order status
  TEST_DECODE_INVALID("ORDER_UPDATE|43|HELLO");
}

void test_decode_invalid_tricky(TestObjs *objs) {
  // When reading integer values, trailing non-digit characters
  // should be considered invalid. Also, the value must be interpreted
  // as base-10.

  TEST_DECODE_INVALID("ORDER_NEW|121x,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
  TEST_DECODE_INVALID("ORDER_NEW|0x123,NEW,121:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");

  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121x:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,0x123:45:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");

  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:45YY:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:0x9:NEW:Dal Makhani:1;121:67:NEW:Samosa:2");

  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:45:NEW:Dal Makhani:1Z;121:67:NEW:Samosa:2");
  TEST_DECODE_INVALID("ORDER_NEW|121,NEW,121:45:NEW:Dal Makhani:0x1;121:67:NEW:Samosa:2");
}

void test_decode_order_new_no_items(TestObjs *objs) {
  try {
    std::string s("ORDER_NEW|473,NEW,");
    Message msg;
    Wire::decode(s, msg);
    FAIL("should throw InvalidMessage decoding Order with no Items");
  } catch (InvalidMessage &msg) {
    // good
  }
}
