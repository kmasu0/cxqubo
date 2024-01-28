#ifndef CXQUBO_MISC_HASHER_H
#define CXQUBO_MISC_HASHER_H

#include <cstddef>
#include <functional>

namespace cxqubo {
template <class T> inline size_t hash_value(const T &v) {
  return std::hash<T>()(v);
}

inline void hash_combine(std::size_t &seed) {}

template <class T, class... Rest>
inline void hash_combine(std::size_t &seed, const T &v, const Rest &...rest) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  hash_combine(seed, rest...);
}

template <class I> inline size_t hash_range(I first, I last) {
  size_t seed = hash_value(*first++);
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
  return seed;
}

template <class I> inline void hash_range(std::size_t &seed, I first, I last) {
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
}

template <class T, class U> inline size_t hash_value(const std::pair<T, U> &p) {
  size_t seed = hash_value(p.first);
  hash_combine(seed, p.second);
  return seed;
}

namespace impl {
template <size_t I, class T>
inline void hash_combine_tuple(size_t &seed, const T &v) {
  if constexpr (I < std::tuple_size<T>::value) {
    hash_combine(seed, hash_value(std::get<I>(v)));
    hash_combine_tuple<I + 1>(seed, v);
  }
}
} // namespace impl

template <class... Ts> inline size_t hash_value(const std::tuple<Ts...> &v) {
  size_t seed = hash_value(std::get<0>(v));
  impl::hash_combine_tuple<1>(seed, v);
  return seed;
}

struct PairHash {
  template <class T, class U>
  size_t operator()(const std::pair<T, U> &p) const {
    return hash_value(p);
  }
};

struct TupleHash {
  template <class... Ts> size_t operator()(const std::tuple<Ts...> &p) const {
    return hash_value(p);
  }
};
} // namespace cxqubo

#endif
