#include <unordered_set>
#include "passwd_db.h"

namespace {

// Note: storing hard-coded credentials in the executable,
// in plain text, would be dangerous and irresponsible for
// a production application. We're doing this here only for
// the sake of simplicity. Please understand this is not
// intended to be an example of how to properly implement
// user authentication in a server.

const std::unordered_set<std::string> s_passwd_db = {
  "admin/password123",
  "testuser/testpass2026",
  "guest/guestaccess",
  "jdoe/qwerty789",
  "dev_account/devmode!",
  "staff_member/orange-juice-44",
  "student01/classroom!",
  "root/supersecret",
  "demo_user/demo-only-99",
  "app_tester/let-me-in-now",
  "alice/foobar",
  "bob/frotz",
};

}

namespace PasswordDB {

bool authenticate(const std::string &cred) {
  return s_passwd_db.find(cred) != s_passwd_db.end();
}

}
