#ifndef CXQUBO_MISC_DRAWABLE_H
#define CXQUBO_MISC_DRAWABLE_H

#include "cxqubo/misc/span.h"
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace cxqubo {
template <class T>
concept Drawable1 = requires(const T x, std::ostream &os) {
  { x.draw(os) } -> std::convertible_to<std::ostream &>;
};

template <Drawable1 T>
inline std::ostream &operator<<(std::ostream &os, const T &x) noexcept {
  return x.draw(os);
}

template <class T>
concept Drawable2 = requires(const T x, std::ostream &os) {
  { draw(os, x) } -> std::convertible_to<std::ostream &>;
};

template <Drawable2 T>
inline std::ostream &operator<<(std::ostream &os, const T &x) noexcept {
  return draw(os, x);
}

template <class T>
concept Drawable3 = requires(const T x, std::ostream &os) {
  { operator<<(os, x) } -> std::convertible_to<std::ostream &>;
};

template <class T>
concept DrawableC = requires {
  Drawable1<T> || Drawable2<T> || Drawable3<T>;
};

//--- Implementation of drawable containers ------------------------------------
namespace impl {
template <size_t I, DrawableC... Args>
inline std::ostream &draw_tuple(std::ostream &os,
                                const std::tuple<Args...> &t) noexcept {
  if constexpr (I >= std::tuple_size<std::tuple<Args...>>::value) {
    return os;
  } else {
    if constexpr (I != 0)
      os << ", ";
    os << std::get<I>(t);
    return draw<I + 1>(os, t);
  }
}
} // namespace impl

template <DrawableC T0, DrawableC T1>
inline std::ostream &operator<<(std::ostream &os,
                                const std::pair<T0, T1> &t) noexcept {
  return os << '(' << t.first << ", " << t.second << ')';
}

template <DrawableC... Args>
inline std::ostream &operator<<(std::ostream &os,
                                const std::tuple<Args...> &t) noexcept {
  os << '(';
  impl::draw_tuple<0>(os, t);
  return os << ')';
}

template <DrawableC V>
inline std::ostream &operator<<(std::ostream &os, ConstSpan<V> span) noexcept {
  os << "[";
  unsigned cnt = 0;
  for (const auto &v : span) {
    if (cnt++ != 0)
      os << ", ";
    os << v;
  }

  return os << "]";
}

template <DrawableC K, DrawableC V, class C, class A>
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

template <DrawableC K, DrawableC V, class H, class P, class A>
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
