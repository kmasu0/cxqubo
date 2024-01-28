#ifndef CXQUBO_MISC_LIST_H
#define CXQUBO_MISC_LIST_H

#include "cxqubo/misc/error_handling.h"
#include <utility>

namespace cxqubo {
template <class T> struct ForwardNode {
  T value = T();
  ForwardNode *next = nullptr;

public:
  ForwardNode() = default;
  ForwardNode(T value) : value(value) {}
  ForwardNode(T value, ForwardNode *next) : value(value), next(next) {}

  ForwardNode(ForwardNode &&arg) { *this = std::move(arg); }
  ForwardNode &operator=(ForwardNode &&arg) {
    if (this != &arg) {
      std::swap(value, arg.value);
      std::swap(next, arg.next);
    }
    return *this;
  }

  ForwardNode(const ForwardNode &) = delete;
  ForwardNode &operator=(const ForwardNode &) = delete;
};

template <class T> struct ForwardNodeIter {
  using Node = ForwardNode<T>;
  Node *node = nullptr;

public:
  ForwardNodeIter() = default;
  ForwardNodeIter(Node *node) : node(node) {}

  friend bool operator==(ForwardNodeIter lhs, ForwardNodeIter rhs) = default;

  T &operator*() {
    assert(node && "iterator out of bounds!");
    return node->value;
  }
  const T &operator*() const {
    assert(node && "iterator out of bounds!");
    return node->value;
  }

  T *operator->() { return &this->operator*(); }
  const T *operator->() const { return &this->operator*(); }

  ForwardNodeIter &operator++() {
    assert(node && "iterator out of bounds!");
    node = node->next;
    return *this;
  }
  ForwardNodeIter operator++(int) {
    auto tmp = *this;
    this->operator++();
    return tmp;
  }

  const Node *next() const { return node->next; }
};
} // namespace cxqubo

#endif
