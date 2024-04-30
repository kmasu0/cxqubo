#ifndef CXQUBO_MISC_COMPARE_H
#define CXQUBO_MISC_COMPARE_H

#include "cxqubo/misc/type_traits.h"
#include <iterator>

namespace cxqubo {
//-- Helper functions ----------------------------------------------------------
template <class L, class R> inline int compare_values(L &&lhs, R &&rhs) {
  return lhs > rhs ? 1 : lhs == rhs ? 0 : -1;
}
template <class L, class R>
inline int compare_values(const std::pair<L, R> &p) {
  return compare_values(p.first, p.second);
}

template <size_t I, class... Ts>
inline int compare_tuple_impl(const std::tuple<Ts...> &lhs,
                              const std::tuple<Ts...> &rhs) {
  int tmp = compare_values(std::get<I>(lhs), std::get<I>(rhs));
  if constexpr (I + 1 == sizeof...(Ts))
    return tmp;
  else
    return tmp != 0 ? tmp : compare_tuple_impl<I + 1>(lhs, rhs);
}

template <class... Ts>
inline int compare_tuple(const std::tuple<Ts...> &lhs,
                         const std::tuple<Ts...> &rhs) {
  return compare_tuple_impl<0>(lhs, rhs);
}

template <class It>
inline int compare_range(It lbegin, It lend, It rbegin, It rend) {
  while (lbegin != lend && rbegin != rend) {
    int tmp = compare_values(*lbegin++, *rbegin++);
    if (tmp != 0)
      return tmp;
  }
  // Compare length.
  return lbegin != lend ? 1 : rbegin == rend ? 0 : -1;
}

template <class C> inline int compare_range(C &&lhs, C &&rhs) {
  return compare_range(std::begin(lhs), std::end(lhs), std::begin(rhs),
                       std::end(rhs));
}

template <class C> inline int compare_range(const C &lhs, const C &rhs) {
  return compare_range(std::begin(lhs), std::end(lhs), std::begin(rhs),
                       std::end(rhs));
}

//-- Operators -----------------------------------------------------------------
template <class T, class Enabler = void>
struct HasCompare : public std::false_type {};

template <class T>
struct HasCompare<
    T, std::enable_if_t<std::is_same_v<int, decltype(std::declval<T>().compare(
                                                std::declval<const T>()))>,
                        void>> : public std::true_type {};

template <class T, class Enabler = void>
struct HasEquals : public std::false_type {};

template <class T>
struct HasEquals<
    T, std::enable_if_t<std::is_same_v<bool, decltype(std::declval<T>().equals(
                                                 std::declval<T>()))>,
                        void>> : public std::true_type {};

template <class T> constexpr bool has_compare = HasCompare<T>::value;
template <class T> constexpr bool has_equals = HasEquals<T>::value;

template <class T> constexpr bool ThreeWayComparable = has_compare<T>;
template <class T>
constexpr bool EqComparable = has_equals<T> || has_compare<T>;

template <class T>
inline std::enable_if_t<EqComparable<T>, bool> operator==(const T &lhs,
                                                          const T &rhs) {
  if constexpr (has_equals<T>)
    return lhs.equals(rhs);
  else
    return lhs.compare(rhs) == 0;
}

template <class T>
inline std::enable_if_t<EqComparable<T>, bool> operator!=(const T &lhs,
                                                          const T &rhs) {
  if constexpr (has_equals<T>)
    return !lhs.equals(rhs);
  else
    return lhs.compare(rhs) != 0;
}

template <class T>
inline std::enable_if_t<ThreeWayComparable<T>, bool> operator>(const T &lhs,
                                                               const T &rhs) {
  return lhs.compare(rhs) > 0;
}
template <class T>
inline std::enable_if_t<ThreeWayComparable<T>, bool> operator>=(const T &lhs,
                                                                const T &rhs) {
  return lhs.compare(rhs) >= 0;
}
template <class T>
inline std::enable_if_t<ThreeWayComparable<T>, bool> operator<(const T &lhs,
                                                               const T &rhs) {
  return lhs.compare(rhs) < 0;
}
template <class T>
inline std::enable_if_t<ThreeWayComparable<T>, bool> operator<=(const T &lhs,
                                                                const T &rhs) {
  return lhs.compare(rhs) <= 0;
}
} // namespace cxqubo

#endif
