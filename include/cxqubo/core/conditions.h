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

#ifndef CXQUBO_CORE_CONDITINOS_H
#define CXQUBO_CORE_CONDITINOS_H

#include "cxqubo/core/entity.h"
#include "cxqubo/misc/hasher.h"
#include "cxqubo/misc/vecmap.h"
#include <vector>

namespace cxqubo {
struct CmpOp {
public:
  enum Kind : uint8_t {
    INVALID = 0,
    EQ = 1, // ==
    NE,     // !=
    GT,     // >
    GE,     // >=
    LT,     // <
    LE,     // <=
  };
  Kind kind = INVALID;

public:
  CmpOp() = default;
  CmpOp(Kind kind) : kind(kind) {}

  static inline CmpOp eq() { return CmpOp(EQ); }
  static inline CmpOp ne() { return CmpOp(NE); }
  static inline CmpOp gt() { return CmpOp(GT); }
  static inline CmpOp ge() { return CmpOp(GE); }
  static inline CmpOp lt() { return CmpOp(LT); }
  static inline CmpOp le() { return CmpOp(LE); }

  int compare(CmpOp rhs) const { return compare_values(kind, rhs.kind); }

  size_t hash() const { return hash_value(kind); }

  template <class T> bool invoke(T lhs, T rhs) const {
    switch (kind) {
    default:
      unreachable_code("invalid logical comparison!");
    case EQ:
      return lhs == rhs;
    case NE:
      return lhs != rhs;
    case GT:
      return lhs > rhs;
    case GE:
      return lhs >= rhs;
    case LT:
      return lhs < rhs;
    case LE:
      return lhs <= rhs;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, CmpOp op) {
    switch (op.kind) {
    default:
      return os << "<invalid>";
    case EQ:
      return os << "==";
    case NE:
      return os << "!=";
    case GT:
      return os << ">";
    case GE:
      return os << ">=";
    case LT:
      return os << "<";
    case LE:
      return os << "<=";
    }
  }
};

inline size_t hash_value(CmpOp v) { return v.hash(); }
} // namespace cxqubo

namespace std {
template <> struct hash<cxqubo::CmpOp> {
  auto operator()(cxqubo::CmpOp v) const noexcept { return v.hash(); }
};
} // namespace std

#endif
