#ifndef CXQUBO_MISC_SPAN_H
#define CXQUBO_MISC_SPAN_H

#include "cxqubo/misc/error_handling.h"
#include <cstring>
#include <initializer_list>
#include <span>
#include <utility>
#include <vector>

namespace cxqubo {
/// Derived from std::span which cannot be modified like LLVM's ArrayRef.
template <class T> class ConstSpan : public std::span<const T> {
  using Super = std::span<const T>;

public:
  using Super::Super;

  /// Ctor for std::vector.
  ConstSpan(const T &elt) : Super(&elt, 1) {}
  /// Ctor for std::vector.
  ConstSpan(const std::vector<T> &vec) : Super(vec.data(), vec.size()) {}
  /// Ctor for std::array.
  template <size_t N>
  ConstSpan(const std::array<T, N> &arr) : Super(arr.data(), N) {}
  /// Ctor for raw array.
  template <size_t N> ConstSpan(const T (&arr)[N]) : Super(arr, N) {}
  /// Ctor for std::span.
  ConstSpan(const std::span<const T> &span) : Super(span.data(), span.size()) {}
  /// Ctor for C const copy.
  ConstSpan(const std::span<T> &span) : Super(span.data(), span.size()) {}
  /// Ctor for std::initializer_list.
  constexpr ConstSpan(const std::initializer_list<T> &init)
      : Super(init.begin() == init.end() ? (T *)nullptr : init.begin(),
              init.size()) {}

  std::vector<T> to_vector() const {
    std::vector<T> result(this->size());
    for (unsigned i = 0, e = this->size(); i != e; ++i)
      result[i] = (*this)[i];
    return result;
  }

  ConstSpan slice(size_t begin, size_t n) const {
    assert(begin + n <= this->size() && "invalid slice specifier");
    return ConstSpan(this->data() + begin, n);
  }
  ConstSpan slice(size_t begin) const {
    return slice(begin, this->size() - begin);
  }
  ConstSpan drop_front(size_t n = 1) const {
    return slice(n, this->size() - n);
  }
  ConstSpan drop_back(size_t n = 1) const { return slice(0, this->size() - n); }
};

template <class T> ConstSpan(const T &arg) -> ConstSpan<T>;
template <class T> ConstSpan(const T *data, size_t len) -> ConstSpan<T>;
template <class T> ConstSpan(const T *begin, const T *end) -> ConstSpan<T>;
template <class T> ConstSpan(const std::vector<T> &vec) -> ConstSpan<T>;
template <class T> ConstSpan(const std::span<T> &arg) -> ConstSpan<T>;
template <class T> ConstSpan(const std::span<const T> &arg) -> ConstSpan<T>;
template <class T, size_t N> ConstSpan(const T (&arr)[N]) -> ConstSpan<T>;

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

  std::span<T> as_span() { return std::span(ptr, n); }
  ConstSpan<T> as_const_span() const { return std::span(ptr, n); }

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

template <class T> inline SpanOwner<T> span_owner(ConstSpan<T> vs) {
  SpanOwner<T> res(vs.size());
  std::copy(vs.begin(), vs.end(), res.data());
  return res;
}

template <class T> inline SpanOwner<T> span_owner(const std::vector<T> &vs) {
  return span_owner(ConstSpan<T>(vs));
}

template <class T>
inline SpanOwner<T> span_owner(const std::initializer_list<T> &vs) {
  return span_owner(ConstSpan<T>(vs));
}

template <class T>
inline bool operator==(const std::span<T> &lhs, const std::span<T> &rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin());
}
} // namespace cxqubo

#endif
