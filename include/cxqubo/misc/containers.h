#ifndef CXQUBO_MISC_CONTAINERS_H
#define CXQUBO_MISC_CONTAINERS_H

#include <algorithm>
#include <numeric>

namespace cxqubo {
template <class C, class T> bool contains(C &&c, T &&v) {
  return c.find(v) != c.end();
}

template <class C, class T> T accumulate(C &&c, T &&init) {
  return std::accumulate(c.begin(), c.end(), std::forward<T>(init));
}
template <class C, class T, class BinOp>
T accumulate(C &&c, T &&init, BinOp binop) {
  return std::accumulate(c.begin(), c.end(), std::forward<T>(init), binop);
}
} // namespace cxqubo

#endif
