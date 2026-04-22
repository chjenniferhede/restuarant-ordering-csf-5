#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>

// Macro to conveniently disable value semantics for a class
#define NO_VALUE_SEMANTICS(ClsName) \
  ClsName(const ClsName&) = delete; \
  ClsName(ClsName&&) = delete; \
  ClsName &operator=(const ClsName &) = delete; \
  ClsName &operator=(ClsName &&) = delete

namespace Util {

template<typename K, typename V>
std::vector<std::pair<V, K>> invert_map(const std::map<K, V> &m) {
  std::vector<std::pair<V, K>> v;
  for (auto i = m.begin(); i != m.end(); ++i)
    v.push_back(std::make_pair(i->second, i->first));
  return v;
}

template<typename K, typename V>
std::vector<std::pair<V, K>> invert_unordered_map(const std::unordered_map<K, V> &m) {
  std::vector<std::pair<V, K>> v;
  for (auto i = m.begin(); i != m.end(); ++i)
    v.push_back(std::make_pair(i->second, i->first));
  return v;
}

// Split a string into components separated by given separator character
inline std::vector<std::string> split(const std::string &s, char sep) {
  std::vector<std::string> v;
  size_t pos = 0;
  bool done = false;

  while (!done) {
    // Find the next occurrence of the separator
    size_t sep_pos = s.find(sep, pos);

    // Extract a token
    std::string tok;
    if (sep_pos == std::string::npos) {
      // There are no more occurrences of the separator,
      // so extract all characters to the end of the string,
      // and this is the last token
      tok = s.substr(pos);
      done = true;
    } else {
      // Extract the characters from the current position to
      // the separator, and continue at the character just past
      // the separator
      tok = s.substr(pos, sep_pos - pos);
      pos = sep_pos + 1;
    }

    // Add token to result
    v.push_back(tok);
  }

  return v;
}

// Trim a string by removing leading and trailing whitespace characters.
inline std::string trim(const std::string &s) {
  std::string::size_type first = s.find_first_not_of(" \t\n\r\f");
  if (first == std::string::npos)
    return "";
  std::string::size_type last = s.find_last_not_of(" \t\n\r\f");
  assert(last < s.size());
  assert(last >= first);
  return s.substr(first, (last - first) + 1);
}


} // end of Util namespace

#endif // UTIL_H
