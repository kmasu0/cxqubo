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

#ifndef CXQUBO_CORE_VARTYPES_H
#define CXQUBO_CORE_VARTYPES_H

#include "cxqubo/misc/error_handling.h"
#include <ostream>
#include <string_view>

namespace cxqubo {
/// Vartype compatible with cimod::Vartype.
enum class Vartype {
  NONE = -1,
  SPIN = 0,
  BINARY = 1,
};

/// String to Vartype.
inline Vartype parse_vartype(std::string_view type) {
  if (type == "SPIN")
    return Vartype::SPIN;
  if (type == "BINARY")
    return Vartype::BINARY;

  return Vartype::NONE;
}

/// Checker.
inline bool is_correct_spin_value(int32_t v, Vartype type) {
  switch (type) {
  default:
    return false;
  case Vartype::SPIN:
    return v == -1 || v == 1;
  case Vartype::BINARY:
    return v == 0 || v == 1;
  }
}

/// Convert a value's type to another.
inline int32_t convert_spin_value(int32_t v, Vartype from, Vartype to) {
  assert(is_correct_spin_value(v, from) && "invalid spin value!");
  if (from == to)
    return v;
  if (from == Vartype::SPIN && to == Vartype::BINARY)
    return (v + 1) / 2;
  if (from == Vartype::BINARY && to == Vartype::SPIN)
    return 2 * v - 1;

  unreachable_code("unknown vartype conversion found!");
}

/// Drawer.
inline std::ostream &draw(std::ostream &os, Vartype type) {
  switch (type) {
  default:
    unreachable_code("invalid variable kind");
  case Vartype::NONE:
    return os << "None";
  case Vartype::BINARY:
    return os << "Binary";
  case Vartype::SPIN:
    return os << "Spin";
  }
}
} // namespace cxqubo

#endif
