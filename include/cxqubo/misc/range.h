#ifndef CXQUBO_MISC_RANGE_H
#define CXQUBO_MISC_RANGE_H

#include "cxqubo/misc/compare.h"
#include "cxqubo/misc/error_handling.h"
#include <utility>

namespace cxqubo {
template <class It> struct Range : public std::pair<It, It> {
  using std::pair<It, It>::pair;

  It begin() { return this->first; }
  It end() { return this->second; }
};

template <class It> inline Range<It> range(It begin_, It end_) {
  return Range<It>{std::move(begin_), std::move(end_)};
}

template <class C> inline auto range(C &&c) {
  return range(std::begin(c), std::end(c));
}

template <class T = int> struct IntIter {
  T cur = T(0);
  T stride = T(1);

public:
  IntIter(T cur = T(0), T stride = T(1)) : cur(cur), stride(stride) {}

  int compare(const IntIter &rhs) const {
    return compare_tuple(std::make_tuple(cur, stride),
                         std::make_tuple(rhs.cur, rhs.stride));
  }

  T &operator*() { return cur; }
  T *operator->() { return &cur; }
  const T &operator*() const { return cur; }
  const T *operator->() const { return &cur; }

  IntIter &operator++() {
    cur += stride;
    return *this;
  }
  IntIter operator++(int) {
    auto tmp = *this;
    this->operator++();
    return tmp;
  }
  IntIter &operator--() {
    cur -= stride;
    return *this;
  }
  IntIter operator--(int) {
    auto tmp = *this;
    this->operator--();
    return tmp;
  }
};

template <class T>
inline Range<IntIter<T>> irange(T begin, T end, T stride = T(1)) {
  static_assert(std::is_integral_v<T>,
                "integer range specifiers must have integer type");
  return range(IntIter<T>(begin, stride), IntIter<T>(end, stride));
}

template <class T> inline Range<IntIter<T>> irange(T end, T stride = T(1)) {
  static_assert(std::is_integral_v<T>,
                "integer range specifiers must have integer type");
  return irange(T(0), end, stride);
}

/// Iterator wrapper like llvm::early_inc_iterator.
template <class T> class DisposableIter {
  T cur = T();
  bool incremented = false;

public:
  DisposableIter(T cur) : cur(cur) {}

  decltype(*std::declval<T>()) operator*() {
    assert(!incremented && "cannot dereference twice!");
    incremented = true;
    return *cur++;
  }

  DisposableIter &operator++() {
    incremented = false;
    return *this;
  }

  friend bool operator==(const DisposableIter &lhs, const DisposableIter &rhs) {
    assert(!lhs.incremented && "cannot compare after incremented!");
    return lhs.cur == rhs.cur;
  }
};

template <class It> inline auto disposable_range(It begin_, It end_) {
  return range<DisposableIter<It>>(DisposableIter(std::move(begin_)),
                                   DisposableIter(std::move(end_)));
}

template <class C> inline auto disposable_range(C &&c) {
  return disposable_range(std::begin(c), std::end(c));
}
} // namespace cxqubo

#endif
