#ifndef CXQUBO_MISC_RANGE_H
#define CXQUBO_MISC_RANGE_H

#include "cxqubo/misc/error_handling.h"
#include <ranges>
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

template <std::ranges::range C> inline auto range(C &&c) {
  return range(std::ranges::begin(c), std::ranges::end(c));
}

template <std::integral T = int> struct IntIter {
  T cur = T(0);
  T stride = T(1);

public:
  IntIter(T cur = T(0), T stride = T(1)) : cur(cur), stride(stride) {}

  friend auto operator<=>(const IntIter &lhs, const IntIter &rhs) = default;

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

template <std::integral T>
inline Range<IntIter<T>> irange(T begin, T end, T stride = T(1)) {
  return range(IntIter<T>(begin, stride), IntIter<T>(end, stride));
}

template <std::integral T>
inline Range<IntIter<T>> irange(T end, T stride = T(1)) {
  return irange(T(0), end, stride);
}

/// Iterator wrapper like llvm::early_inc_iterator.
template <std::forward_iterator T> class DisposableIter {
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

template <std::ranges::range C> inline auto disposable_range(C &&c) {
  return disposable_range(std::ranges::begin(c), std::ranges::end(c));
}
} // namespace cxqubo

#endif
