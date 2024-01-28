#ifndef CXQUBO_MISC_VECMAP_H
#define CXQUBO_MISC_VECMAP_H

#include "cxqubo/misc/error_handling.h"
#include <iostream>
#include <vector>

namespace cxqubo {
template <class Key>
concept VecMapKey = requires(Key key, size_t index) {
  { key.index() } -> std::convertible_to<std::size_t>;
  { Key::from(index) } -> std::same_as<Key>;
};

template <VecMapKey Key, class T> class VecMap {
protected:
  using Vec = std::vector<T>;
  Vec vec;
  T nil = T();

public:
  VecMap() = default;
  VecMap(const T &nil) : nil(nil) {}

  bool inbounds(Key key) const { return key.index() < vec.size(); }

  typename Vec::size_type size() const { return vec.size(); }

  typename Vec::reference operator[](Key key) {
    assert(inbounds(key) && "index out of bounds!");
    return vec[key.index()];
  }
  typename Vec::const_reference operator[](Key key) const {
    assert(inbounds(key) && "index out of bounds!");
    return vec[key.index()];
  }

public:
  Key append(const T &value) {
    Key key = Key::from(this->vec.size());
    vec.push_back(value);
    return key;
  }

  Key append(T &&value) {
    Key key = Key::from(this->vec.size());
    vec.push_back(std::move(value));
    return key;
  }

  void resize(typename Vec::size_type size) { vec.resize(size, nil); }
  void grow(Key key) {
    typename Vec::size_type newsize = key.index() + 1;
    if (newsize > vec.size())
      resize(newsize);
  }

  void clear() { vec.clear(); }
};
} // namespace cxqubo

#endif
