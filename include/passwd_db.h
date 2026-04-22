#ifndef PASSWD_DB_H
#define PASSWD_DB_H

#include <string>

namespace PasswordDB {

// Determine whether a login credential (from the string value
// in a LOGIN message) is valid. Returns true if yes, false if no.
bool authenticate(const std::string &cred);

}


#endif // PASSWD_DB_H
