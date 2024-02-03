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

#ifndef CXQUBO_CORE_VARIABLES_H
#define CXQUBO_CORE_VARIABLES_H

#include "cxqubo/core/vartypes.h"
#include "cxqubo/misc/drawable.h"
#include <string_view>

namespace cxqubo {
/// Variable data.
struct VariableData {
  std::string_view name;
  Vartype type;

  std::ostream &draw(std::ostream &os) const {
    return os << type << "(" << name << ")";
  }

  bool equals(const VariableData &rhs) const {
    return name == rhs.name && type == rhs.type;
  }
};
} // namespace cxqubo

#endif
