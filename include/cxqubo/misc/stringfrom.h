#ifndef CXQUBO_MISC_STRINGFROM_H
#define CXQUBO_MISC_STRINGFROM_H

#include "cxqubo/misc/drawable.h"
#include <sstream>
#include <string>

namespace cxqubo {
template <DrawableC T>
inline std::string string_from(const T &obj) {
  std::stringstream ss;
  ss << obj;
  return ss.str();
}
} // namespace cxqubo

#endif
