#ifndef CXQUBO_MISC_SPANREF_H
#define CXQUBO_MISC_SPANREF_H

#include "cxqubo/misc/compare.h"
#include "cxqubo/misc/error_handling.h"
#include <array>
#include <cstring>
#include <initializer_list>
#include <utility>
#include <vector>

namespace cxqubo {
/// Just like std::span in C++20.
template <class T> class SpanRef {
public:
  using value_type = T;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  using iterator = const T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

private:
  const T *ptr = nullptr;
  size_t n = 0;

public:
  SpanRef() = default;
  SpanRef(const SpanRef &arg) : ptr(arg.ptr), n(arg.n) {}
  SpanRef &operator=(const SpanRef &arg) {
    if (this != &arg) {
      ptr = arg.ptr;
      n = arg.n;
    }
    return *this;
  }

  /// Ctor normal construction.
  SpanRef(const T *ptr, size_t n) : ptr(ptr), n(n) {}
  /// Ctor for one element.
  SpanRef(const T &elt) : ptr(&elt), n(1) {}
  /// Ctor for iterators.
  SpanRef(const T *begin, const T *end) : ptr(begin), n(end - begin) {}
  /// Ctor for std::vector.
  SpanRef(const std::vector<T> &vec) : ptr(vec.data()), n(vec.size()) {}
  /// Ctor for std::array.
  template <size_t N>
  SpanRef(const std::array<T, N> &arr) : ptr(arr.data()), n(arr.size()) {}
  /// Ctor for raw array.
  template <size_t N> SpanRef(const T (&arr)[N]) : ptr(arr), n(N) {}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winit-list-lifetime"
#endif
  /// Ctor for std::initializer_list.
  SpanRef(const std::initializer_list<T> &init)
      : ptr(init.begin() == init.end() ? (const T *)nullptr : init.begin()),
        n(init.size()) {}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

public:
  const T *data() const { return ptr; }
  size_t size() const { return n; }

  bool empty() const { return n == 0; }

  iterator begin() const { return ptr; }
  iterator end() const { return ptr + n; }

  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }

  const T &operator[](size_t i) const {
    assert(i < n && "index out of bounds!");
    return ptr[i];
  }

  const T &front() const {
    asssert(!empty());
    return *ptr;
  }
  const T &back() const {
    asssert(!empty());
    return ptr[n - 1];
  }

  bool equals(SpanRef rhs) const {
    return n == rhs.n && std::equal(begin(), end(), rhs.begin());
  }

  std::vector<T> vec() const {
    std::vector<T> result(this->size());
    for (unsigned i = 0, e = this->size(); i != e; ++i)
      result[i] = (*this)[i];
    return result;
  }

  SpanRef slice(size_t begin, size_t n) const {
    assert(begin + n <= this->size() && "invalid slice specifier");
    return SpanRef(this->data() + begin, n);
  }
  SpanRef slice(size_t begin) const {
    return slice(begin, this->size() - begin);
  }
  SpanRef drop_front(size_t n = 1) const { return slice(n, this->size() - n); }
  SpanRef drop_back(size_t n = 1) const { return slice(0, this->size() - n); }
};

// SpanRef deductions

template <class T> SpanRef(const T &v) -> SpanRef<T>;
template <class T> SpanRef(const T *begin, const T *end) -> SpanRef<T>;
template <class T> SpanRef(const std::vector<T> &v) -> SpanRef<T>;
template <class T, size_t N> SpanRef(const std::array<T, N> &v) -> SpanRef<T>;
template <class T, size_t N> SpanRef(const T (&v)[N]) -> SpanRef<T>;
template <class T> SpanRef(const std::initializer_list<T> &init) -> SpanRef<T>;
template <class T> SpanRef(const SpanRef<T> &arg) -> SpanRef<T>;
template <class T> SpanRef(SpanRef<T> &arg) -> SpanRef<T>;

/// Owner of span.
template <class T> class SpanOwner {
  T *ptr = nullptr;
  size_t n = 0;

public:
  SpanOwner() = default;
  SpanOwner(size_t n) { resize(n); }
  SpanOwner(size_t n, const T &v) {
    resize(n);
    set_all(v);
  }
  ~SpanOwner() { remove(); }

  SpanOwner(SpanOwner &&arg) { *this = std::move(arg); }
  SpanOwner &operator=(SpanOwner &&rhs) {
    if (this != &rhs) {
      std::swap(ptr, rhs.ptr);
      std::swap(n, rhs.n);
    }
    return *this;
  }

  SpanRef<T> as_spanref() const { return SpanRef<T>(ptr, n); }

  T *data() { return ptr; }
  const T *data() const { return ptr; }
  size_t size() const { return n; }
  size_t bufsize() const { return n * sizeof(T); }

  const T &operator[](size_t i) const {
    assert(i < n && "index out of bounds!");
    return ptr[i];
  }
  T &operator[](size_t i) {
    assert(i < n && "index out of bounds!");
    return ptr[i];
  }

  SpanOwner clone() const {
    SpanOwner result(n);
    std::memcpy(result.ptr, ptr, bufsize());
    return result;
  }

public:
  void set_all(const T &v) {
    for (unsigned i = 0; i != n; ++i)
      ptr[i] = v;
  }
  void resize(size_t new_n) {
    remove();
    if (new_n == 0) {
      ptr = nullptr;
      n = 0;
    } else {
      ptr = new T[new_n];
      n = new_n;
    }
  }
  void remove() {
    if (ptr && n)
      delete[] ptr;
    ptr = nullptr;
    n = 0;
  }
};

template <class T> inline SpanOwner<T> span_owner(SpanRef<T> vs) {
  SpanOwner<T> res(vs.size());
  std::copy(vs.begin(), vs.end(), res.data());
  return res;
}

template <class T> inline SpanOwner<T> span_owner(const std::vector<T> &vs) {
  return span_owner(SpanRef<T>(vs));
}

template <class T>
inline SpanOwner<T> span_owner(const std::initializer_list<T> &vs) {
  return span_owner(SpanRef<T>(vs));
}

} // namespace cxqubo

#endif
