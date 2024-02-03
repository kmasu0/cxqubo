#ifndef CXQUBO_MISC_CTOR_H
#define CXQUBO_MISC_CTOR_H

#include "cxqubo/misc/type_traits.h"
#include <utility>

namespace cxqubo {
template <class T, class... Args> inline T make(Args &&...args) {
  if constexpr (std::is_constructible_v<T, Args...>) {
    return T(std::forward<Args>(args)...);
  } else {
    return T{std::forward<Args>(args)...};
  }
}
} // namespace cxqubo

#endif
