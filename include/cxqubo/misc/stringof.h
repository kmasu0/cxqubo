#ifndef CXQUBO_MISC_STRINGOF_H
#define CXQUBO_MISC_STRINGOF_H

#include <sstream>
#include <string>

namespace cxqubo {
template <class T> inline std::string stringof(const T &obj) {
  std::stringstream ss;
  ss << obj;
  return ss.str();
}
} // namespace cxqubo

#endif
