#ifndef CXQUBO_MISC_SHAPE_H
#define CXQUBO_MISC_SHAPE_H

#include "cxqubo/misc/containers.h"
#include "cxqubo/misc/error_handling.h"
#include "cxqubo/misc/math.h"
#include "cxqubo/misc/range.h"
#include "cxqubo/misc/spanref.h"
#include <algorithm>
#include <numeric>
#include <vector>

namespace cxqubo {
/// Array strides.
using ArrayStrides = SpanRef<unsigned>;
/// Array indexes.
using ArrayIndexes = SpanRef<size_t>;

/// Array shape.
struct ArrayShape : public SpanRef<unsigned> {
  using Super = SpanRef<unsigned>;

public:
  using Super::Super;
  using Super::operator=;

  ArrayShape(const SpanRef<unsigned> &arg) : Super(arg) {}
  ArrayShape &operator=(const SpanRef<unsigned> &arg) {
    static_cast<Super &>(*this) = arg;
    return *this;
  }

  int compare(const ArrayShape &rhs) const { return compare_range(*this, rhs); }

  size_t nelements() const {
    return accumulate(*this, 1, [](size_t acc, size_t n) { return acc * n; });
  }

  bool inbounds(ArrayIndexes indexes) const {
    assert(indexes.size() <= size() && "index out of bounds!");

    for (unsigned i = 0, e = indexes.size(); i != e; ++i)
      if (indexes[i] >= (*this)[i])
        return false;

    return true;
  }

  std::vector<unsigned> strides() const {
    unsigned n = size();
    std::vector<unsigned> result(n);
    result[n - 1] = 1;
    for (unsigned i = 1; i != n; ++i) {
      unsigned j = n - 1 - i;
      result[j] = (*this)[j + 1] * result[j + 1];
    }
    return result;
  }

  unsigned offset(ArrayIndexes indexes) const {
    assert(inbounds(indexes) && "index out of bounds!");

    unsigned v = 0;
    unsigned n = indexes.size();
    const auto sts = strides();
    for (unsigned i = 0; i != n; ++i) {
      unsigned j = n - 1 - i;
      v += indexes[j] * sts[j];
    }
    return v;
  }

  template <class T> T *addressof(T *p, ArrayIndexes indexes) const {
    return p + offset(indexes);
  }
};

class ArrayShapeIter {
  ArrayShape shape;
  std::vector<size_t> indexes;

public:
  ArrayShapeIter(ArrayShape shape, bool is_end)
      : shape(shape), indexes(shape.size() + 1, 0) {
    if (is_end)
      indexes[0] = 1;
  }
  ArrayShapeIter(ArrayShape shape, ArrayIndexes is) : shape(shape) {
    assert(shape.size() == is.size() && "indexes must be same!");
    assert(shape.inbounds(is) && "indexes out of bounds!");
    indexes.emplace_back(0);
    indexes.insert(indexes.end(), is.begin(), is.end());
  }

  ArrayIndexes operator*() const { return ArrayIndexes(indexes).drop_front(); }

  bool equals(const ArrayShapeIter &rhs) const {
    return shape == rhs.shape && indexes == rhs.indexes;
  }

  ArrayShapeIter &operator++() {
    advance();
    return *this;
  }

  ArrayShapeIter operator++(int) {
    auto tmp = *this;
    this->operator++();
    return tmp;
  }

  bool is_end() const { return indexes[0] == 1; }

private:
  void advance() {
    assert(!is_end() && "index out of bounds!");

    unsigned i = shape.size();
    for (; i != 0 && indexes[i] == shape[i - 1] - 1; --i)
      indexes[i] = 0;

    indexes[i] += 1;
  }
};

inline Range<ArrayShapeIter> shape_range(ArrayShape shape) {
  return range(ArrayShapeIter(shape, false), ArrayShapeIter(shape, true));
}

template <class T>
constexpr bool is_shaped_array =
    std::is_convertible_v<decltype(std::declval<T>().shape()), ArrayShape>
        &&std::is_convertible_v<decltype(std::declval<T>().ndim()), size_t>
            &&std::is_convertible_v<decltype(std::declval<T>().remain(
                                        std::declval<unsigned>())),
                                    T>;

template <class A> struct ShapedIter {
  const A *array = nullptr;
  unsigned index = 0;

public:
  ShapedIter() = default;
  ShapedIter(const A *array, unsigned index) : array(array), index(index) {
    static_assert(is_shaped_array<A>, "range of array must be shaped array!");
    assert(array->ndim() != 0 && "Array cannot be iterated!");
  }

  bool is_end() const { return index >= array->shape()[0]; }

  A operator*() const {
    assert(!is_end() && "iterator out of bounds!");
    return array->remain(index);
  }

  int compare(const ShapedIter &rhs) const {
    return compare_values(std::make_tuple(array, index),
                          std::make_tuple(rhs.array, rhs.index));
  }

  ShapedIter &operator++() {
    assert(!is_end() && "iterator out of bounds!");
    ++index;
    return *this;
  }
  ShapedIter operator++(int) {
    auto tmp = *this;
    this->operator++();
    return tmp;
  }

  std::ostream &draw(std::ostream &os) const {
    return os << "ShapedIter(" << index << ")";
  }
};

template <class A> inline Range<ShapedIter<A>> array_range(const A &array) {
  return range(ShapedIter<A>(&array, 0),
               ShapedIter<A>(&array, array.shape()[0]));
}

inline std::vector<size_t> parse_indexes(std::string_view s) {
  std::vector<size_t> vec;
  const char *data = s.data();
  unsigned i = 0, e = s.size();
  while (i != e) {
    assert(data[i] == '[' && "wrong string");
    ++i;

    assert(isdigit(data[i]) && "wrong string");
    size_t n = 0;
    while (isdigit(data[i])) {
      n = n * 10 + size_t(data[i] - '0');
      ++i;
    }
    vec.emplace_back(n);

    assert(data[i] == ']' && "wrong string");
    ++i;
  }

  return vec;
}
} // namespace cxqubo

#endif
