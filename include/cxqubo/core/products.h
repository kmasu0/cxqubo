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

#ifndef CXQUBO_CORE_PRODUCTS_H
#define CXQUBO_CORE_PRODUCTS_H

#include "cxqubo/core/entity.h"
#include "cxqubo/core/variables.h"
#include "cxqubo/misc/compare.h"
#include "cxqubo/misc/drawable.h"
#include "cxqubo/misc/hasher.h"
#include "cxqubo/misc/spanref.h"

namespace cxqubo {
class ProductData : public SpanRef<Variable> {
  using Super = SpanRef<Variable>;

public:
  ProductData() = default;
  ProductData(const SpanRef<Variable> &arg) : Super(arg) {}
  ProductData &operator=(const SpanRef<Variable> &arg) {
    static_cast<Super &>(*this) = arg;
    return *this;
  }

  size_t hash() const { return hash_range(begin(), end()); }

  std::ostream &draw(std::ostream &os) const {
    os << '(';
    unsigned cnt = 0;
    for (auto var : *this) {
      if (cnt++ != 0)
        os << ", ";
      os << var;
    }
    return os << ')';
  }

  int compare(const ProductData &rhs) const {
    return compare_range(*this, rhs);
  }
};

inline size_t hash_value(ProductData product) { return product.hash(); }
} // namespace cxqubo

namespace std {
template <> struct hash<cxqubo::ProductData> {
  auto operator()(cxqubo::ProductData v) const noexcept { return v.hash(); }
};
} // namespace std

#endif
