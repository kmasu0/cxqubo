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

#ifndef CXQUBO_CORE_POLYNOMIAL_H
#define CXQUBO_CORE_POLYNOMIAL_H

#include "cxqubo/core/context.h"
#include "cxqubo/misc/debug.h"
#include "cxqubo/misc/error_handling.h"
#include <unordered_map>

namespace cxqubo {
/// A polynomial with multiple terms.
using Multi = std::unordered_map<Product, Expr>;
/// A polynomial with a single term.
using Single = std::pair<Product, Expr>;

class ConstPolyIter {
  Variant<std::monostate, Single, Multi::const_iterator> iter;
  Multi::const_iterator end;

public:
  ConstPolyIter() = default;
  ConstPolyIter(Single single) : iter(single) {}
  ConstPolyIter(Multi::const_iterator it, Multi::const_iterator end)
      : iter(it), end(end) {}

  bool equals(const ConstPolyIter &rhs) const {
    return iter == rhs.iter && end == rhs.end;
  }

  std::pair<Product, Expr> operator*() const {
    assert(!iter.empty() && "index out of bounds!");
    if (auto *p = iter.as_ptr_if<Single>())
      return *p;
    else
      return *iter.as<Multi::const_iterator>();
  }

  ConstPolyIter &operator++() {
    advance();
    return *this;
  }

  ConstPolyIter operator++(int) {
    auto tmp = *this;
    this->operator++();
    return tmp;
  }

private:
  void advance();
};

class Poly : public Variant<std::monostate, Single, Multi> {
  using Super = Variant<std::monostate, Single, Multi>;

public:
  using Super::Super;
  using Super::operator=;

  static inline Product term_none() { return Product::none(); }

  bool is_empty() const { return is<std::monostate>(); }
  bool is_single() const { return is<Single>(); }
  bool is_multi() const { return is<Multi>(); }

  size_t size() const {
    if (is_empty())
      return 0;
    else if (is_single())
      return 1;
    return as<Multi>().size();
  }

  ConstPolyIter begin() const {
    if (auto *p = as_ptr_if<Single>())
      return *p;
    if (auto *p = as_ptr_if<Multi>())
      return ConstPolyIter(p->begin(), p->end());

    return ConstPolyIter();
  }

  ConstPolyIter end() const { return ConstPolyIter(); }

  friend std::ostream &operator<<(std::ostream &os, const Poly &v) {
    if (const auto *p = v.as_ptr_if<Single>())
      return os << '{' << p->first << ", " << p->second << '}';
    if (const auto *p = v.as_ptr_if<Multi>())
      return os << *p;
    return os << "<none>";
  }

public:
  void clear() { *this = std::monostate(); }
  void insert_or_add(Context &ctx, Product term, Expr coeff) {
    // Single.
    if (auto *p = as_ptr_if<Single>()) {
      if (p->first == term) {
        p->second = ctx.add(p->second, coeff);
      } else {
        *this = Multi{as<Single>(), {term, coeff}};
      }
    } else if (auto *p = as_ptr_if<Multi>()) {
      auto [it, inserted] = p->emplace(term, coeff);
      if (!inserted)
        it->second = ctx.add(it->second, coeff);
    } else {
      *this = Single{term, coeff};
    }
  }
};

inline void ConstPolyIter::advance() {
  assert(!iter.empty() && "index out of bounds!");

  if (iter.is<Single>())
    iter = std::monostate();
  else {
    auto it = ++iter.as<Multi::const_iterator>();
    if (it == end)
      iter = std::monostate();
  }
}

/// Generator of polynomial expressions.
/// TODO: Implement and use memory recycler for Poly.
struct PolyBuilder {
  Context *ctx = nullptr;

public:
  PolyBuilder(Context &ctx) : ctx(&ctx) {}

public:
  bool is_constant(const Poly &poly) const {
    return poly.is_single() && !poly.as<Single>().first;
  }

  bool is_a_variable(const Poly &poly) const {
    return poly.is_single() && ctx->dim_of(poly.as<Single>().first) == 1;
  }

  Expr constant_value(const Poly &poly) const {
    return is_constant(poly) ? poly.as<Single>().second : Expr::none();
  }

  Variable a_variable(const Poly &poly) const {
    return is_a_variable(poly) ? ctx->product_data(poly.as<Single>().first)[0]
                               : Variable::none();
  }

public:
  Poly variable(Variable var) {
    auto term = ctx->save_product({var}, true);
    auto type = ctx->var_data(var).type;
    if (type == Vartype::SPIN) {
      return Multi{{term, ctx->fp(2.0)}, {Poly::term_none(), ctx->fp(-1)}};
    } else if (type == Vartype::BINARY) {
      return Single{term, ctx->fp(1.0)};
    }
    unreachable_code("unsupported variable type!");
  }

  Poly constant(Expr coeff) const { return Single{Poly::term_none(), coeff}; }

  void neg_assign(Poly &poly) const {
    if (auto *p = poly.as_ptr_if<Single>()) {
      p->second = ctx->neg(p->second);
      return;
    }

    if (auto *p = poly.as_ptr_if<Multi>()) {
      for (auto &[term, coeff] : *p)
        coeff = ctx->neg(coeff);
    }
  }
  void add_assign(Poly &lhs, const Poly &rhs) const {
    if (auto *p = rhs.as_ptr_if<Single>()) {
      lhs.insert_or_add(*ctx, p->first, p->second);
    } else {
      for (auto [term, coeff] : rhs.as<Multi>())
        lhs.insert_or_add(*ctx, term, coeff);
    }
  }
  void mul_assign(Poly &lhs, const Poly &rhs) {
    if (const auto *lp = lhs.as_ptr_if<Single>()) {
      if (const auto *rp = rhs.as_ptr_if<Single>())
        lhs = mul_single_single(*lp, *rp);
      else if (const auto *rp = rhs.as_ptr_if<Multi>())
        lhs = mul_multi_single(*rp, *lp);
      else
        unreachable_code("unable to multiply polys");
    } else if (const auto *rp = rhs.as_ptr_if<Single>()) {
      lhs = mul_multi_single(lhs.as<Multi>(), *rp);
    } else {
      lhs = mul_multi_multi(lhs.as<Multi>(), rhs.as<Multi>());
    }
  }

private:
  inline Product mul_terms(Product lhs, Product rhs) {
    if (lhs && rhs)
      return ctx->mul_products(lhs, rhs);
    else if (lhs)
      return lhs;
    else if (rhs)
      return rhs;
    else
      return Poly::term_none();
  }

  Poly mul_single_single(const Single &lhs, const Single &rhs) {
    return Single{mul_terms(lhs.first, rhs.first),
                  ctx->mul(lhs.second, rhs.second)};
  }

  Poly mul_multi_single(const Multi &lhs, const Single &rhs) {
    Poly result;
    if (is_constant(rhs)) {
      result = lhs;
      for (auto &[term, coeff] : result.as<Multi>())
        coeff = ctx->mul(coeff, rhs.second);
      return result;
    } else {
      for (auto [lterm, lcoeff] : lhs)
        result.insert_or_add(*ctx, mul_terms(lterm, rhs.first),
                             ctx->mul(lcoeff, rhs.second));
      return result;
    }
  }

  Poly mul_multi_multi(const Multi &lhs, const Multi &rhs) {
    Poly result;
    for (auto [lterm, lcoeff] : lhs)
      for (auto [rterm, rcoeff] : rhs)
        result.insert_or_add(*ctx, mul_terms(lterm, rterm),
                             ctx->mul(lcoeff, rcoeff));
    return result;
  }
};
} // namespace cxqubo

#endif
