#ifndef CXQUBO_MISC_STRINGOF_H
#define CXQUBO_MISC_STRINGOF_H

#include "cxqubo/misc/drawable.h"
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
