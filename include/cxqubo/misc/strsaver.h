#ifndef CXQUBO_MISC_STRSAVER_H
#define CXQUBO_MISC_STRSAVER_H

#include "cxqubo/misc/allocator.h"
#include <set>
#include <string>
#include <string_view>

namespace cxqubo {
using StringAllocator = TypeBumpAllocator<std::string>;

class StringSaver {
  StringAllocator &alloc;
  std::set<std::string_view> strings;

public:
  StringSaver(StringAllocator &alloc) : alloc(alloc) {}

  std::string_view save_string(std::string_view str) {
    if (str.empty())
      return "";

    auto it = strings.find(str);
    if (it == strings.end()) {
      str = new_string(str);
      auto [it2, flag] = strings.insert(str);
      it = it2;
    }
    return *it;
  }

  std::string_view new_string(std::string_view str) {
    return *(new (alloc.allocate()) std::string{str});
  }

  bool contains(std::string_view str) const { return strings.contains(str); }
};
} // namespace cxqubo

#endif
