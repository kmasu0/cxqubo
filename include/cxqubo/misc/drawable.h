#ifndef CXQUBO_MISC_DRAWABLE_H
#define CXQUBO_MISC_DRAWABLE_H

#include "cxqubo/misc/spanref.h"
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace cxqubo {
//--- Helper functions ---------------------------------------------------------
template <class It>
inline std::ostream &draw_range(std::ostream &os, It begin, It end) {
  unsigned cnt = 0;
  while (begin != end) {
    if (cnt++ != 0)
      os << ", ";
    os << *begin++;
  }
  return os;
}

template <class C> inline std::ostream &draw_range(std::ostream &os, C &&c) {
  return draw_range(os, std::begin(c), std::end(c));
}

namespace impl {
template <size_t I, class... Args>
inline std::ostream &draw_tuple(std::ostream &os,
                                const std::tuple<Args...> &t) noexcept {
  if constexpr (I >= std::tuple_size<std::tuple<Args...>>::value) {
    return os;
  } else {
    if constexpr (I != 0)
      os << ", ";
    os << std::get<I>(t);
    return draw_tuple<I + 1>(os, t);
  }
}
} // namespace impl

template <class... Args>
inline std::ostream &draw_tuple(std::ostream &os,
                                const std::tuple<Args...> &t) noexcept {
  return impl::draw_tuple<0>(os, t);
}

template <class T0, class T1>
inline std::ostream &draw_pair(std::ostream &os, const std::pair<T0, T1> &t,
                               const char *delimitor = ", ") noexcept {
  return os << '(' << t.first << delimitor << t.second << ')';
}

//--- Implementation of drawable containers ------------------------------------
template <class T0, class T1>
inline std::ostream &operator<<(std::ostream &os,
                                const std::pair<T0, T1> &t) noexcept {
  return os << '(' << t.first << ", " << t.second << ')';
}

template <class... Args>
inline std::ostream &operator<<(std::ostream &os,
                                const std::tuple<Args...> &t) noexcept {
  os << '(';
  draw_tuple(os, t);
  return os << ')';
}

template <class V>
inline std::ostream &operator<<(std::ostream &os,
                                const SpanRef<V> &span) noexcept {
  os << "[";
  draw_range(os, span);
  return os << "]";
}

template <class K, class V, class C, class A>
inline std::ostream &operator<<(std::ostream &os,
                                const std::map<K, V, C, A> &m) noexcept {
  os << "{";

  unsigned cnt = 0;
  for (const auto &[k, v] : m) {
    if (cnt++ != 0)
      os << ' ';
    os << k << ": " << v;
    if (cnt != m.size())
      os << ",\n";
  }

  return os << "}";
}

template <class K, class V, class H, class P, class A>
inline std::ostream &
operator<<(std::ostream &os,
           const std::unordered_map<K, V, H, P, A> &m) noexcept {
  os << "{";

  unsigned cnt = 0;
  for (const auto &[k, v] : m) {
    if (cnt++ != 0)
      os << ' ';
    os << k << ": " << v;
    if (cnt != m.size())
      os << ",\n";
  }

  return os << "}";
}
} // namespace cxqubo

#endif
