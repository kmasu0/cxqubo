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

#ifndef CXQUBO_CORE_EXPRESS_H
#define CXQUBO_CORE_EXPRESS_H

#include "cxqubo/core/conditions.h"
#include "cxqubo/core/context.h"
#include "cxqubo/misc/hasher.h"
#include "cxqubo/misc/shape.h"
#include <sstream>

namespace cxqubo {
/// Interface of generating and manipulating an expression.
struct Express {
public:
  /// CmpOp comparison.
  using Cmp = std::tuple<Express, CmpOp, double>;

public:
  Context *ctx = nullptr;
  Expr ref;

public:
  Express(Context *ctx, Expr ref) : ctx(ctx), ref(ref) {}

  bool equals(const Express &rhs) const {
    return ctx == rhs.ctx && ref == rhs.ref;
  }

  ExprData data() const { return ctx->expr_data(ref); }

  size_t hash() const {
    size_t seed = hash_value(ctx);
    hash_combine(seed, ref);
    return seed;
  }

  std::string_view name() const { return ctx->expr_name(ref); }

public:
  std::ostream &draw_as_tree(std::ostream &os) const {
    return ctx->draw_tree(os, ref);
  }
  std::string as_tree() const {
    std::stringstream ss;
    draw_as_tree(ss);
    return ss.str();
  }

public:
  Express pow(int n) const {
    if (n <= 0)
      unreachable_code("`exponent` should be positive.");

    Express result = *this;
    while (--n > 0)
      result = result * (*this);
    return result;
  }
  Express neg() const { return Express(ctx, ctx->neg(ref)); }

public:
  friend Express operator+(const Express &lhs, const Express &rhs) {
    return Express(lhs.ctx, lhs.ctx->add(lhs.ref, rhs.ref));
  }
  friend Express operator+(const Express &lhs, double rhs) {
    return Express(lhs.ctx, lhs.ctx->add(lhs.ref, lhs.ctx->fp(rhs)));
  }
  friend Express operator+(double lhs, const Express &rhs) {
    return Express(rhs.ctx, rhs.ctx->add(rhs.ctx->fp(lhs), rhs.ref));
  }
  friend Express &operator+=(Express &lhs, const Express &rhs) {
    return lhs = lhs + rhs;
  }
  friend Express &operator+=(Express &lhs, double rhs) {
    return lhs = lhs + rhs;
  }

  friend Express operator-(const Express &lhs, const Express &rhs) {
    return Express(lhs.ctx, lhs.ctx->sub(lhs.ref, rhs.ref));
  }
  friend Express operator-(const Express &lhs, double rhs) {
    return Express(lhs.ctx, lhs.ctx->sub(lhs.ref, lhs.ctx->fp(rhs)));
  }
  friend Express operator-(double lhs, const Express &rhs) {
    return Express(rhs.ctx, rhs.ctx->sub(rhs.ctx->fp(lhs), rhs.ref));
  }
  friend Express &operator-=(Express &lhs, const Express &rhs) {
    return lhs = lhs - rhs;
  }
  friend Express &operator-=(Express &lhs, double rhs) {
    return lhs = lhs - rhs;
  }

  friend Express operator*(const Express &lhs, const Express &rhs) {
    return Express(lhs.ctx, lhs.ctx->mul(lhs.ref, rhs.ref));
  }
  friend Express operator*(const Express &lhs, double rhs) {
    return Express(lhs.ctx, lhs.ctx->mul(lhs.ref, lhs.ctx->fp(rhs)));
  }
  friend Express operator*(double lhs, const Express &rhs) {
    return Express(rhs.ctx, rhs.ctx->mul(rhs.ctx->fp(lhs), rhs.ref));
  }
  friend Express &operator*=(Express &lhs, const Express &rhs) {
    return lhs = lhs * rhs;
  }
  friend Express &operator*=(Express &lhs, double rhs) {
    return lhs = lhs * rhs;
  }

  friend Express operator/(const Express &lhs, double rhs) {
    if (rhs == 0)
      unreachable_code("zero divide error.");

    return Express(lhs.ctx, lhs.ctx->mul(lhs.ref, lhs.ctx->fp(1 / rhs)));
  }
  friend Express &operator/=(Express &lhs, double rhs) {
    return lhs = lhs / rhs;
  }

  friend Cmp operator==(const Express &lhs, double rhs) {
    return {lhs, CmpOp::EQ, rhs};
  }
  friend Cmp operator!=(const Express &lhs, double rhs) {
    return {lhs, CmpOp::NE, rhs};
  }
  friend Cmp operator>(const Express &lhs, double rhs) {
    return {lhs, CmpOp::GT, rhs};
  }
  friend Cmp operator>=(const Express &lhs, double rhs) {
    return {lhs, CmpOp::GE, rhs};
  }
  friend Cmp operator<(const Express &lhs, double rhs) {
    return {lhs, CmpOp::LT, rhs};
  }
  friend Cmp operator<=(const Express &lhs, double rhs) {
    return {lhs, CmpOp::LE, rhs};
  }
};

inline std::ostream &operator<<(std::ostream &os, const Express &e) {
  return e.ctx->draw_expr(os, e.ref);
}

inline std::ostream &operator<<(std::ostream &os, const Express::Cmp &cmp) {
  const auto &[expr, op, value] = cmp;
  return os << '(' << expr << ' ' << op << ' ' << value << ')';
}

inline size_t hash_value(const Express &v) { return v.hash(); }

/// Return a SubH from the given expression.
inline Express subh(Express expr, std::string_view label) {
  return Express(expr.ctx, expr.ctx->subh(label, expr.ref));
}
/// Return a Constraint from the given expression and conditional function.
inline Express constraint(Express expr, Condition cond,
                          std::string_view label) {
  auto *ctx = expr.ctx;
  return Express(ctx, ctx->constraint(label, expr.ref, cond));
}
/// Return a Constraint from the given logical expression as follows.
/// auto x = model.binary();
inline Express constraint(const Express::Cmp &cmp, std::string_view label) {
  auto [lhs, op, rhs] = cmp;
  return constraint(lhs, lhs.ctx->insert_cmp(op, rhs), label);
}
/// Return a Constraint from the given expression and checking an energy of
/// the expression is zero or not.
inline Express constraint(Express expr, std::string_view label) {
  return constraint(expr == 0.0, label);
}

class Array {
  Context *ctx = nullptr;
  Expr base_;
  ArrayShape shape_;

public:
  using Item = Express;

public:
  Array() = default;
  Array(Context *ctx, Expr base_, ArrayShape shape_)
      : ctx(ctx), base_(base_), shape_(shape_) {}
  Array(Express expr) : Array(expr.ctx, expr.ref, {}) {}

  size_t ndim() const { return shape_.size(); }
  ArrayShape shape() const { return shape_; }

  size_t size() const { return ndim() == 0 ? 1 : shape_[0]; }
  size_t size_at(unsigned i) const {
    assert(ndim() != 0 && "Array has no elements!");
    assert(i < shape_.size() && "index out of bounds!");
    return shape_[i];
  }
  size_t nelements() const { return ndim() == 0 ? 1 : shape_.nelements(); }

  bool equals(const Array &rhs) const {
    if (ctx != rhs.ctx || base_ != rhs.base_)
      return false;

    if (shape_.size() != rhs.shape_.size())
      return false;

    if (ndim() == 0)
      return true;

    return shape_ == rhs.shape_;
  }

  Express base() const { return Express(ctx, base_); }
  Express at(ArrayIndexes indexes) const {
    return Express(ctx, Expr::from(base_.index() + shape_.offset(indexes)));
  }

  Express at_offset(unsigned offset) const {
    assert(ndim() != 0 && "Array has no elements!");
    assert(offset < nelements() && "offset out of bounds!");
    return Express(ctx, Expr::from(base_.index() + offset));
  }

  /// Access element. Unlike multi dimentional array, this class cannot access
  /// each element directly by operator[].
  Express operator*() const { return Express(ctx, base_); }
  Express operator->() const { return Express(ctx, base_); }

  /// Index access as followings.
  ///
  /// Array array2d = ...; // 2D array.
  /// for (unsigned i = 0; i != array.size_at(0); ++i) {
  ///   for (unsigned j = 0; j != array.size_at(1); ++j) {
  ///     // Access by each index for each dimention.
  ///     auto e0 = *array2d[i][j];
  ///     ...
  ///     // Access by all indexes at once.
  ///     auto e1 = *array2d[{i, j}];
  ///     ...
  ///   }
  /// }
  Array operator[](ArrayIndexes indexes) const { return remain(indexes); }
  /// Implementation of operator[].
  Array remain(ArrayIndexes indexes) const {
    assert(shape_.size() > 0 && "Array has no elements!");
    auto b = Expr::from(base_.index() + shape_.offset(indexes));
    return Array(ctx, b, ArrayShape(shape_.drop_front(indexes.size())));
  }

  size_t hash() const {
    size_t seed = hash_value(ctx);
    hash_combine(seed, base_);
    if (shape_.size() == 0)
      hash_combine(seed, nullptr);
    else
      hash_range(seed, shape_.begin(), shape_.end());
    return seed;
  }

  friend std::ostream &operator<<(std::ostream &os, const Array &arr) {
    os << "Array(";
    arr.draw_impl(os, 0, arr.ndim(), 6, true);
    return os << ")\n";
  }

public:
  friend Express operator+(const Array &lhs, const Array &rhs) {
    return (*lhs) + (*rhs);
  }
  friend Express operator+(const Array &lhs, double rhs) {
    return (*lhs) + rhs;
  }
  friend Express operator+(double lhs, const Array &rhs) {
    return lhs + (*rhs);
  }
  friend Express &operator+=(Express &lhs, const Array &rhs) {
    return lhs += *rhs;
  }

  friend Express operator-(const Array &lhs, const Array &rhs) {
    return (*lhs) - (*rhs);
  }
  friend Express operator-(const Array &lhs, double rhs) {
    return (*lhs) - rhs;
  }
  friend Express operator-(double lhs, const Array &rhs) {
    return lhs - (*rhs);
  }
  friend Express &operator-=(Express &lhs, const Array &rhs) {
    return lhs -= *rhs;
  }

  friend Express operator*(const Array &lhs, const Array &rhs) {
    return (*lhs) * (*rhs);
  }
  friend Express operator*(const Array &lhs, double rhs) {
    return (*lhs) * rhs;
  }
  friend Express operator*(double lhs, const Array &rhs) {
    return lhs * (*rhs);
  }
  friend Express &operator*=(Express &lhs, const Array &rhs) {
    return lhs *= *rhs;
  }

  friend Express operator/(const Array &lhs, double rhs) {
    return (*lhs) / rhs;
  }

  friend Express::Cmp operator==(const Array &lhs, double rhs) {
    return *lhs == rhs;
  }
  friend Express::Cmp operator!=(const Array &lhs, double rhs) {
    return *lhs != rhs;
  }
  friend Express::Cmp operator>(const Array &lhs, double rhs) {
    return *lhs > rhs;
  }
  friend Express::Cmp operator>=(const Array &lhs, double rhs) {
    return *lhs >= rhs;
  }
  friend Express::Cmp operator<(const Array &lhs, double rhs) {
    return *lhs < rhs;
  }
  friend Express::Cmp operator<=(const Array &lhs, double rhs) {
    return *lhs <= rhs;
  }

public:
  using iterator = ShapedIter<Array>;
  iterator begin() const {
    assert(ndim() != 0 && "Scalar has no elements!");
    return iterator(this, 0);
  }
  iterator end() const {
    assert(ndim() != 0 && "Scalar has no elements!");
    return iterator(this, shape_[0]);
  }

  using index_iterator = IntIter<size_t>;
  index_iterator index_begin() const {
    assert(ndim() != 0 && "Scalar has no indexes!");
    return index_iterator();
  }
  index_iterator index_end() const {
    assert(ndim() != 0 && "Scalar has no indexes!");
    return index_iterator(shape_[0]);
  }
  Range<index_iterator> indexes() const {
    return range(index_begin(), index_end());
  }

  using array_index_iterator = ArrayShapeIter;
  array_index_iterator array_index_begin() const {
    assert(ndim() != 0 && "Scalar has no indexes!");
    return array_index_iterator(shape_, false);
  }
  array_index_iterator array_index_end() const {
    assert(ndim() != 0 && "Scalar has no indexes!");
    return array_index_iterator(shape_, true);
  }
  Range<array_index_iterator> array_indexes() const {
    return range(array_index_begin(), array_index_end());
  }

private:
  void draw_impl(std::ostream &os, unsigned depth, unsigned whole_dim,
                 unsigned indent, bool is_first) const {
    if (ndim() == 0) {
      if (!is_first)
        os << ", ";

      os << base();
      return;
    }

    if (depth != 0) {
      if (!is_first) {
        os << ',';
        os << std::string(whole_dim - depth, '\n');
        // "Array(" + "["*
        os << std::string(indent + depth, ' ');
      }
    }

    os << '[';
    for (unsigned i = 0, e = shape_[0]; i != e; ++i) {
      remain(i).draw_impl(os, depth + 1, whole_dim, indent, i == 0);
    }
    os << ']';
  }
};

inline size_t hash_value(const Array &v) { return v.hash(); }
} // namespace cxqubo

namespace std {
template <> struct hash<cxqubo::Express> {
  auto operator()(cxqubo::Express v) const noexcept { return v.hash(); }
};

template <> struct hash<cxqubo::Array> {
  auto operator()(const cxqubo::Array &v) const noexcept { return v.hash(); }
};
} // namespace std

#endif
