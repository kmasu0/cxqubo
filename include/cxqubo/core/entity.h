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

#ifndef CXQUBO_CORE_ENTITY_H
#define CXQUBO_CORE_ENTITY_H

#include "cxqubo/misc/compare.h"
#include "cxqubo/misc/error_handling.h"
#include "cxqubo/misc/hasher.h"
#include <functional>
#include <ostream>

namespace cxqubo {
/// Entity in CXQUBO which satisfies VecMapKey concept.
template <class SubClass, class T, char PREFIX> class Entity {
  static inline constexpr T INVALID = 0;

  T id = INVALID;

protected:
  Entity(T i) : id(i) {
    static_assert(std::is_base_of<Entity, SubClass>::value,
                  "Must pass the derived type to this template!");
    static_assert(std::is_unsigned_v<T>,
                  "Entity ID type must be unsigned integral!");
  }

public:
  Entity() = default;

  static inline SubClass from(size_t index) {
    assert(index < size_t(~T(0)) && "index out of bounds!");
    return SubClass{Entity(T(index + 1))};
  }
  static inline SubClass raw_from(T id) {
    assert(id <= ~T(0) && "index out of bounds!");
    return SubClass{Entity(T(id))};
  }

  static inline constexpr SubClass none() { return SubClass{}; }

public:
  inline operator bool() const { return valid(); }

  int compare(Entity rhs) const { return compare_values(id, rhs.id); }

  size_t index() const { return id - 1; }
  size_t raw_id() const { return id; }

  bool is_none() const { return id == INVALID; }
  bool valid() const { return id != INVALID; }

  size_t hash() const { return hash_value(id); }

  std::ostream &draw(std::ostream &os) const {
    if (valid())
      return os << PREFIX << index();
    else
      return os << PREFIX << "(invalid)";
  }
};

/// A reference of an expression in tree format.
struct Expr : public Entity<Expr, unsigned, 'e'> {};
/// A reference of a variable.
struct Variable : public Entity<Variable, unsigned, 'v'> {};
/// A reference of a product of variables.
struct Product : public Entity<Product, unsigned, 'p'> {};
/// A reference of a condition check function.
struct Condition : public Entity<Condition, unsigned, 'c'> {};
/// Check condition of the constraints.
using ConditionFn = std::function<bool(double)>;

inline size_t hash_value(cxqubo::Expr v) { return v.hash(); }
inline size_t hash_value(cxqubo::Variable v) { return v.hash(); }
inline size_t hash_value(cxqubo::Condition v) { return v.hash(); }
inline size_t hash_value(cxqubo::Product v) { return v.hash(); }
} // namespace cxqubo

namespace std {
template <> struct hash<cxqubo::Expr> {
  auto operator()(cxqubo::Expr v) const noexcept { return v.hash(); }
};
template <> struct hash<cxqubo::Variable> {
  auto operator()(cxqubo::Variable v) const noexcept { return v.hash(); }
};
template <> struct hash<cxqubo::Condition> {
  auto operator()(cxqubo::Condition v) const noexcept { return v.hash(); }
};
template <> struct hash<cxqubo::Product> {
  auto operator()(cxqubo::Product v) const noexcept { return v.hash(); }
};
} // namespace std

#endif
