#ifndef CXQUBO_MISC_VECMAP_H
#define CXQUBO_MISC_VECMAP_H

#include "cxqubo/misc/error_handling.h"
#include <iostream>
#include <vector>

namespace cxqubo {
template <class Key>
constexpr bool is_vecmap_key =
    std::is_convertible_v<decltype(std::declval<Key>().index()), size_t>
        &&std::is_convertible_v<decltype(Key::from(std::declval<size_t>())),
                                size_t>;

template <class Key, class T> class VecMap {
protected:
  using Vec = std::vector<T>;
  Vec vec;
  T nil = T();

public:
  VecMap() = default;
  VecMap(const T &nil) : nil(nil) {
    static_assert(is_vecmap_key<Key>, "1st template argument of VecMap must "
                                      "satisfy is_vecmap_key requirement!");
  }

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
