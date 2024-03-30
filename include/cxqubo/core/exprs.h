/// Copyright 2024 Koichi Masuda
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.

#ifndef CXQUBO_CORE_EXPRS_H
#define CXQUBO_CORE_EXPRS_H

#include "cxqubo/core/entity.h"
#include "cxqubo/misc/compare.h"
#include "cxqubo/misc/debug.h"
#include "cxqubo/misc/error_handling.h"
#include "cxqubo/misc/list.h"
#include "cxqubo/misc/variant.h"

namespace cxqubo {
/// Floating point value.
struct Fp {
  double value = 0.0;

  friend std::ostream &operator<<(std::ostream &os, Fp v) {
    return os << std::to_string(v.value);
  }
  int compare(Fp rhs) const { return compare_values(value, rhs.value); }
};

/// Placeholder value.
struct Placeholder {
  std::string_view name;
  friend std::ostream &operator<<(std::ostream &os, Placeholder v) {
    return os << "place('" << v.name << "')";
  }
  int compare(const Placeholder &rhs) const {
    return compare_values(name, rhs.name);
  }
};

/// Labeled expression.
struct SubH {
  std::string_view label;
  Expr expr;

  friend std::ostream &operator<<(std::ostream &os, const SubH &v) {
    return os << "subh"
              << "('" << v.label << "', " << v.expr << ")";
  }
  int compare(const SubH &rhs) const {
    return compare_tuple(std::make_tuple(label, expr),
                         std::make_tuple(rhs.label, expr));
  }
};

/// Labeled expression.
struct Constraint {
  std::string_view label;
  Expr expr;
  Condition cond;

  friend std::ostream &operator<<(std::ostream &os, const Constraint &v) {
    return os << "constr"
              << "('" << v.label << "', " << v.expr << ")";
  }
  int compare(const Constraint &rhs) const {
    return compare_tuple(std::make_tuple(label, expr, cond),
                         std::make_tuple(rhs.label, rhs.expr, rhs.cond));
  }
};

enum class Op : uint8_t {
  INVALID = 0,
  // Unary.
  Neg, // '-'
  // Binary
  Sub, // '-'
  // List
  Add, // '+'
  Mul, // '*'
};

inline std::ostream &operator<<(std::ostream &os, Op op) {
  switch (op) {
  case Op::INVALID:
    return os << "<invalid>";
  case Op::Neg:
    return os << '-';
  case Op::Sub:
    return os << '-';
  case Op::Add:
    return os << '+';
  case Op::Mul:
    return os << '*';
  }
  unreachable_code("unknown operation!");
}

struct Unary {
  Op op = Op::INVALID;
  Expr operand;

  friend std::ostream &operator<<(std::ostream &os, const Unary &v) {
    return os << "(" << v.op << v.operand << ')';
  }
  bool equals(const Unary &rhs) const {
    return op == rhs.op && operand == rhs.operand;
  }
};

struct List {
  using Node = ForwardNode<Expr>;
  using iterator = ForwardNodeIter<Expr>;
  using const_iterator = const ForwardNodeIter<Expr>;

  Op op = Op::INVALID;
  Node *node = nullptr;

  friend std::ostream &operator<<(std::ostream &os, const List &v) {
    os << '(' << v.node->value;

    auto *n = v.node->next;
    while (n) {
      os << ' ' << v.op << ' ' << n->value;
      n = n->next;
    }
    return os << ')';
  }

  bool equals(const List &rhs) const {
    return op == rhs.op && node == rhs.node;
  }

  iterator begin() { return iterator(node); }
  iterator end() { return iterator(); }

  const_iterator begin() const { return const_iterator(node); }
  const_iterator end() const { return const_iterator(); }
};

/// Variant of expression.
using ExprVariant = Variant<std::monostate, Fp, Variable, Placeholder, SubH,
                            Constraint, Unary, List>;
struct ExprData : public ExprVariant {
  using Super = ExprVariant;
  using Super::Super;
  using Super::operator=;

  ExprData(const ExprData &arg) { *this = arg; }
  ExprData &operator=(const ExprData &arg) {
    if (this != &arg)
      std::visit([this](auto x) { *this = x; }, arg);
    return *this;
  }

  struct Drawer {
    std::ostream &os;
    std::ostream &operator()(const Fp &v) { return os << v; }
    std::ostream &operator()(const Variable &v) { return os << v; }
    std::ostream &operator()(const Placeholder &v) { return os << v; }
    std::ostream &operator()(const SubH &v) { return os << v; }
    std::ostream &operator()(const Constraint &v) { return os << v; }
    std::ostream &operator()(const Unary &v) { return os << v; }
    std::ostream &operator()(const List &v) { return os << v; }
    std::ostream &operator()(std::monostate) { return os << "<invalid>"; }
  };
  friend std::ostream &operator<<(std::ostream &os, const ExprData &v) {
    return std::visit(Drawer{os}, v);
  }
};
} // namespace cxqubo

#endif
