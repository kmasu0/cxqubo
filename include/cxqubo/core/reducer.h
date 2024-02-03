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

#ifndef CXQUBO_CORE_REDUCER_H
#define CXQUBO_CORE_REDUCER_H

#include "cxqubo/core/context.h"
#include "cxqubo/core/poly.h"

namespace cxqubo {
inline constexpr double DEFAULT_STRENGTH = 5.0;

#if 0
template <class T>
concept TermCoeffInserter = requires(T x, ConstSpan<Variable> term,
                                     double coeff) {
  { x.insert_or_add(term, coeff) } -> std::same_as<void>;
  { x.ignore(term, coeff) } -> std::convertible_to<bool>;
};
#endif

template <class T>
constexpr bool is_termcoeff_inserter =
    std::is_same_v<decltype(std::declval<T>().insert_or_add(
                       std::declval<SpanRef<Variable>>(),
                       std::declval<double>())),
                   void>
        &&std::is_convertible_v<decltype(std::declval<T>().ignore(
                                    std::declval<SpanRef<Variable>>(),
                                    std::declval<double>())),
                                bool>;

template <class Inserter> class LimitedInserter {
  Context &ctx;
  Inserter &inserter;
  double strength;
  size_t limit = 2;

public:
  LimitedInserter(Context &ctx, Inserter &inserter, double strength,
                  size_t limit = 2)
      : ctx(ctx), inserter(inserter), strength(strength), limit(2) {
    static_assert(is_termcoeff_inserter<Inserter>,
                  "template argument must satisfy is_termcoeff_inserter!");
  }

  /// Create new variables q[0:dim-limit] and convert
  ///   x_0 * x_1 * ... * x_(dim-1)
  /// ->
  ///   x_(dim-1) * .. * x_(dim-limit-2) * q_0 +
  ///   Hc(x_0, x_1, q_0) +
  ///   Hc(x_2, q_0, q_1) +
  ///   ...
  ///   Hc(x_(dim+limit-4), q_(dim+limit-6), q_(dim+limit-5))
  ///
  /// For example, when limit=2, convert
  ///   xyz
  /// ->
  ///   zq +  3q + xy - 2yq - 2qx
  std::vector<Variable> redce_and_insert(Product term, double coeff) {
    /// Constant or dimention of term is small enough.
    if (ctx.dim_of(term) <= limit) {
      insert_or_add(term, coeff);
      return {};
    }

    return redce_and_insert_impl(term, coeff);
  }
  /// Insert A * Hc(q, x, y) = A * (3q + xy - 2yq - 2qx)
  void insert_Hc(Variable q, Variable x, Variable y, double A) {
    auto xy = ctx.save_product({x, y}, false);
    auto xq = ctx.save_product({x, q}, false);
    auto yq = ctx.save_product({y, q}, false);
    insert_or_add(ctx.save_product({q}, true), 3.0 * A * strength);
    insert_or_add(xy, A * strength);
    insert_or_add(xq, -2.0 * A * strength);
    insert_or_add(yq, -2.0 * A * strength);
  }

private:
  std::vector<Variable> redce_and_insert_impl(Product term, double coeff) {
    auto xs = ctx.product_data(term);
    if (inserter.ignore(xs, coeff))
      return {};

    auto dim = xs.size();
    // Create q[0:dim-limit].
    auto qs = ctx.create_unnamed_vars(dim - limit, Vartype::BINARY);

    // x_(dim-1) * .. * x_(dim-limit-2) * q_0
    std::vector<Variable> vars(limit);
    for (unsigned i = 0; i != limit - 1; ++i)
      vars[i] = xs[dim - i - 1];
    vars[limit - 1] = qs[0];
    insert_or_add(ctx.save_product(vars, true), coeff);

    // Hc(x_0, x_1, q_0)
    insert_Hc(qs[0], xs[0], xs[1], coeff);

    if (dim >= limit + 2) {
      // limit >= 2 and dim >= 4, so dim + limit - 6 >= 0
      for (unsigned i = 0, e = dim + limit - 6; i < e; ++i)
        insert_Hc(qs[i + 1], qs[i], xs[i + 2], coeff);
    }

    return qs;
  }

  void insert_or_add(Product term, double coeff) {
    inserter.insert_or_add(ctx.product_data(term), coeff);
  }
};
} // namespace cxqubo

#endif
