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
#include "cxqubo/misc/debug.h"
#include "cxqubo/misc/drawable.h"
#include "cxqubo/misc/error_handling.h"
#include "cxqubo/misc/list.h"
#include "cxqubo/misc/variant.h"

namespace cxqubo {
/// Floating point value.
struct Fp {
  double value = 0.0;

  std::ostream &draw(std::ostream &os) const {
    return os << std::to_string(value);
  }
  friend auto operator<=>(Fp lhs, Fp rhs) = default;
};

/// Placeholder value.
struct Placeholder {
  std::string_view name;
  std::ostream &draw(std::ostream &os) const {
    return os << "place('" << name << "')";
  }
  friend auto operator<=>(const Placeholder &lhs,
                          const Placeholder &rhs) = default;
};

/// Labeled expression.
struct SubH {
  std::string_view label;
  Expr expr;

  std::ostream &draw(std::ostream &os) const {
    return os << "subh"
              << "('" << label << "', " << expr << ")";
  }
  friend auto operator<=>(const SubH &lhs, const SubH &rhs) = default;
};

/// Labeled expression.
struct Constraint {
  std::string_view label;
  Expr expr;
  Condition cond;

  std::ostream &draw(std::ostream &os) const {
    return os << "constr"
              << "('" << label << "', " << expr << ")";
  }
  friend auto operator<=>(const Constraint &lhs,
                          const Constraint &rhs) = default;
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

inline std::ostream &draw(std::ostream &os, Op op) {
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

  std::ostream &draw(std::ostream &os) const {
    return os << "(" << op << operand << ')';
  }
  friend bool operator==(const Unary &lhs, const Unary &rhs) = default;
};

struct List {
  using Node = ForwardNode<Expr>;
  using iterator = ForwardNodeIter<Expr>;
  using const_iterator = const ForwardNodeIter<Expr>;

  Op op = Op::INVALID;
  Node *node = nullptr;

  std::ostream &draw(std::ostream &os) const {
    os << '(' << node->value;

    auto *n = node->next;
    while (n) {
      os << ' ' << op << ' ' << n->value;
      n = n->next;
    }
    return os << ')';
  }

  friend bool operator==(const List &lhs, const List &rhs) = default;

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
  std::ostream &draw(std::ostream &os) const {
    return std::visit(Drawer{os}, *this);
  }
};
} // namespace cxqubo

#endif
