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

#ifndef CXQUBO_CORE_CONTEXT_H
#define CXQUBO_CORE_CONTEXT_H

#include "cxqubo/core/conditions.h"
#include "cxqubo/core/entity.h"
#include "cxqubo/core/exprs.h"
#include "cxqubo/core/products.h"
#include "cxqubo/core/sample.h"
#include "cxqubo/core/variables.h"
#include "cxqubo/misc/allocator.h"
#include "cxqubo/misc/compiler.h"
#include "cxqubo/misc/ctor.h"
#include "cxqubo/misc/strsaver.h"
#include "cxqubo/misc/vecmap.h"
#include <set>
#include <unordered_map>

namespace cxqubo {
/// This is important class for using CXQUBO. It owns and manages basic
/// enetities in CXQUBO, including the variable, expression and product uniquing
/// tables.
class Context {
  StringAllocator stralloc;
  StringSaver strsaver;
  // Expression data.
  VecMap<Expr, ExprData> exprs;
  std::unordered_map<double, Expr> fpconsts;
  std::unordered_map<std::string_view, Expr> placeholders;
  TypeBumpAllocator<List::Node> node_allocator;
  // Variable data.
  VecMap<Variable, VariableData> vars;
  std::map<std::string_view, Variable> name_to_ref;
  // Product data.
  std::unordered_map<ProductData, Product> data_to_product;
  VecMap<Product, SpanOwner<Variable>> products;
  // Condition data.
  VecMap<Condition, std::pair<CmpOp, double>> cmps;
  std::unordered_map<std::pair<CmpOp, double>, Condition, PairHash> cmp_to_cond;

public:
  Context() : strsaver(stralloc) { insert_cmp(CmpOp::EQ, 0.0); }
  Context(const Context &) = delete;
  Context &operator=(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator=(Context &&) = delete;

  ExprData expr_data(Expr expr) const { return exprs[expr]; }
  VariableData var_data(Variable var) const { return vars[var]; }
  ProductData product_data(Product p) const {
    return p ? products[p].as_const_span() : ProductData();
  }

  bool contains_var(std::string_view name) const {
    return name_to_ref.contains(name);
  }

  Variable var_of(std::string_view name) const {
    auto it = name_to_ref.find(name);
    assert(it != name_to_ref.end() && "name has not been registered!");
    return it->second;
  }

  Variable expr_var(Expr expr) const {
    auto data = expr_data(expr);
    if (auto *p = data.as_ptr_if<Variable>())
      return *p;
    return Variable::none();
  }

  std::string_view expr_name(Expr expr) const {
    auto data = expr_data(expr);
    if (auto *p = data.as_ptr_if<Variable>())
      return var_data(*p).name;
    if (auto *p = data.as_ptr_if<Placeholder>())
      return p->name;
    if (auto *p = data.as_ptr_if<SubH>())
      return p->label;
    if (auto *p = data.as_ptr_if<Constraint>())
      return p->label;
    return "";
  }

  unsigned dim_of(Product p) const { return p ? product_data(p).size() : 0; }

  bool contains_cmp(CmpOp op, double rhs) const {
    return cmp_to_cond.contains({op, rhs});
  }
  size_t num_cmps() const { return cmp_to_cond.size(); }

  /// Checking 0 or not.
  Condition eqz() const { return Condition::from(0); }

  bool apply_cond(Condition cond, double lhs) const {
    auto [op, rhs] = cmps[cond];
    return op.invoke(lhs, rhs);
  }

  Sample convert_sample(const Sample &sample, Vartype vtype) const {
    Sample result;
    for (auto [id, spin] : sample) {
      auto origin = var_data(Variable::from(id)).type;
      result.emplace(id, convert_spin_value(spin, vtype, origin));
    }
    return result;
  }

public:
  std::string_view save_string(std::string_view s) {
    return strsaver.save_string(s);
  }

  Variable create_var(std::string_view name, Vartype type) {
    if (name.empty())
      return create_unnamed_var(type);

    assert(name_to_ref.count(name) == 0 &&
           "a variable with same name is found!");

    name = strsaver.save_string(name);
    auto var = vars.append({name, type});
    name_to_ref[name] = var;

    debug_code(odbg_indent() << var << " = '" << name << "'\n");

    return var;
  }

  Variable create_unnamed_var(Vartype type) {
    auto var = vars.append({"", type});
    debug_code(odbg_indent() << var << " = '<unnamed>'\n");
    return var;
  }
  std::vector<Variable> create_unnamed_vars(unsigned n, Vartype type) {
    std::vector<Variable> result(n);
    for (unsigned i = 0; i != n; ++i)
      result[i] = create_unnamed_var(type);
    return result;
  }

  Product mul_products(Product l, Product r) {
    if (dim_of(l) == 0)
      return r;
    if (dim_of(r) == 0)
      return l;

    auto lhs = product_data(l);
    auto rhs = product_data(r);
    if (lhs.size() == 1 && rhs.size() == 1)
      return lhs[0] <= rhs[0] ? save_product({lhs[0], rhs[0]}, true)
                              : save_product({rhs[0], lhs[0]}, true);

    std::vector<Variable> vars(lhs.begin(), lhs.end());
    vars.insert(vars.end(), rhs.begin(), rhs.end());
    std::sort(vars.begin(), vars.end());

    return save_product(vars, true);
  }

  Product save_product(ConstSpan<Variable> vars, bool is_sorted = false) {
    if (vars.empty())
      return Product::none();

    std::vector<Variable> tmp;
    if (!is_sorted) {
      tmp.insert(tmp.end(), vars.begin(), vars.end());
      std::sort(tmp.begin(), tmp.end());
      vars = tmp;
    }

    auto it = data_to_product.find(vars);
    if (it != data_to_product.end())
      return it->second;

    Product p = products.append(span_owner(vars));
    auto data = ProductData(products[p].as_const_span());
    data_to_product[data] = p;

    debug_code(odbg_indent() << p << " = " << data << '\n');

    return p;
  }

  Expr fp(double value) {
    auto it = fpconsts.find(value);
    if (it != fpconsts.end())
      return it->second;

    auto expr = insert_expr(make<Fp>(value));
    fpconsts[value] = expr;
    return expr;
  }

  Expr variable(Variable var) {
    auto expr = insert_expr(var);
    return expr;
  }
  std::vector<Expr> variables(ConstSpan<Variable> vs) {
    std::vector<Expr> result;
    for (auto v : vs)
      result.emplace_back(variable(v));
    return result;
  }
  Expr placeholder(std::string_view name) {
    assert(!name.empty() && "placeholder must have non-empty name");
    auto it = placeholders.find(name);
    if (it != placeholders.end())
      return it->second;

    name = strsaver.save_string(name);
    auto expr = insert_expr(make<Placeholder>(name));
    placeholders[name] = expr;
    return expr;
  }

  Expr subh(std::string_view label, Expr expr) {
    label = strsaver.save_string(label);
    return insert_expr(make<SubH>(label, expr));
  }
  Expr constraint(std::string_view label, Expr expr, Condition cond) {
    label = strsaver.save_string(label);
    return insert_expr(make<Constraint>(label, expr, cond));
  }

  Expr neg(Expr expr) {
    if (auto e = constfold_unary(Op::Neg, expr))
      return e;

    return insert_expr(make<Unary>(Op::Neg, expr));
  }

  Expr add(Expr lhs, Expr rhs) { return binlist(Op::Add, lhs, rhs); }
  Expr sub(Expr lhs, Expr rhs) { return add(lhs, neg(rhs)); }
  Expr mul(Expr lhs, Expr rhs) { return binlist(Op::Mul, lhs, rhs); }

  Condition insert_cmp(CmpOp op, double rhs) {
    auto cond = cmps.append({op, rhs});
    cmp_to_cond.emplace(std::make_pair(op, rhs), cond);
    return cond;
  }

public:
  std::ostream &draw_variable(std::ostream &os, Variable var) const {
    return os << var_data(var);
  }

  std::ostream &draw_product(std::ostream &os, Product product) const {
    os << '(';

    if (product) {
      unsigned cnt = 0;
      for (auto var : product_data(product)) {
        if (cnt++ != 0)
          os << ", ";
        os << "'" << var_data(var).name << "'";
      }
    }

    return os << ')';
  }

  std::ostream &draw_expr(std::ostream &os, Expr expr) const {
    auto data = expr_data(expr);
    if (auto *p = data.as_ptr_if<Variable>()) {
      return os << var_data(*p);
    } else if (auto *p = data.as_ptr_if<SubH>()) {
      os << p->label << '(';
      return draw_expr(os, p->expr) << ')';
    } else if (auto *p = data.as_ptr_if<Constraint>()) {
      os << p->label << '(' << p->cond << ", ";
      return draw_expr(os, p->expr) << ')';
    } else if (auto *p = data.as_ptr_if<Unary>()) {
      os << p->op;
      return draw_expr(os, p->operand);
    } else if (auto *p = data.as_ptr_if<List>()) {
      os << '(';
      auto *n = p->node;
      draw_expr(os, n->value);
      while (n->next) {
        n = n->next;
        os << ' ' << p->op << ' ';
        draw_expr(os, n->value);
      }
      return os << ')';
    } else {
      return os << data;
    }
  }

  std::ostream &draw_tree(std::ostream &os, Expr expr) const {
    os << '<' << expr << ">\n";
    return draw_tree_impl(os, expr, "", false);
  }

  // Debug methods.
  CXQUBO_DUMP_METHOD void dump(Variable v) const {
    draw_variable(odbg(), v) << '\n';
  }
  CXQUBO_DUMP_METHOD void dump(Expr v) const { draw_expr(odbg(), v) << '\n'; }
  CXQUBO_DUMP_METHOD void dump(Product v) const {
    draw_product(odbg(), v) << '\n';
  }
  CXQUBO_DUMP_METHOD void dump_var(unsigned id) const {
    dump(Variable::raw_from(id));
  }
  CXQUBO_DUMP_METHOD void dump_expr(unsigned id) const {
    dump(Expr::raw_from(id));
  }
  CXQUBO_DUMP_METHOD void dump_product(unsigned id) const {
    dump(Product::raw_from(id));
  }

private:
  std::ostream &draw_tree_impl(std::ostream &os, Expr expr,
                               const std::string &prefix, bool is_left) const {
    auto data = expr_data(expr);

    std::string next_prefix = prefix + (is_left ? "│  " : "   ");
    os << prefix << (is_left ? "├──" : "└──");

    if (auto *p = data.as_ptr_if<Variable>()) {
      return os << var_data(*p).name << '\n';
    } else if (auto *p = data.as_ptr_if<SubH>()) {
      os << "subh('" << p->label << "')\n";
      return draw_tree_impl(os, p->expr, next_prefix, false);
    } else if (auto *p = data.as_ptr_if<Constraint>()) {
      os << "constr('" << p->label << "', " << p->cond << ")\n";
      return draw_tree_impl(os, p->expr, next_prefix, false);
    } else if (auto *p = data.as_ptr_if<Unary>()) {
      os << p->op << '\n';
      return draw_tree_impl(os, p->operand, next_prefix, false);
    } else if (auto *p = data.as_ptr_if<List>()) {
      os << p->op << '\n';
      auto *n = p->node;
      while (n) {
        draw_tree_impl(os, n->value, next_prefix, n->next);
        n = n->next;
      }
      return os;
    } else {
      return draw_expr(os, expr) << "\n";
    }
  }

  Expr binlist(Op op, Expr lhs, Expr rhs) {
    if (auto e = constfold_binary(op, lhs, rhs))
      return e;

    // new_node:lhs -> node:rhs
    auto rhs_data = expr_data(rhs);
    if (auto rhs_p = rhs_data.as_ptr_if<List>()) {
      if (rhs_p->op == op) {
        auto *n = node_allocator.create(lhs, rhs_p->node);
        return insert_expr(make<List>(op, n));
      }
    }

    // new_node:rhs -> node:lhs
    auto lhs_data = expr_data(lhs);
    if (auto lhs_p = lhs_data.as_ptr_if<List>()) {
      if (lhs_p->op == op) {
        auto *n = node_allocator.create(rhs, lhs_p->node);
        return insert_expr(make<List>(op, n));
      }
    }

    // new_node:lhs -> new_node:rhs
    auto *next = node_allocator.create(rhs);
    auto *n = node_allocator.create(lhs, next);
    return insert_expr(make<List>(op, n));
  }

  Expr insert_expr(const ExprData &data) {
    auto expr = exprs.append(data);
    debug_code(odbg_indent() << expr << " = " << data << '\n');
    return expr;
  }

  Expr constfold_unary(Op op, Expr operand) {
    auto data = expr_data(operand);
    auto *ep = data.as_ptr_if<Fp>();
    if (ep) {
      switch (op) {
      case Op::Neg:
        return fp(-ep->value);
      default:
        break;
      }
    }

    return Expr::none();
  }
  Expr constfold_binary(Op op, Expr lhs, Expr rhs) {
    auto lhs_data = expr_data(lhs);
    auto rhs_data = expr_data(rhs);
    auto *lhs_p = lhs_data.as_ptr_if<Fp>();
    auto *rhs_p = rhs_data.as_ptr_if<Fp>();

    if (lhs_p && rhs_p) {
      if (op == Op::Add)
        return fp(lhs_p->value + rhs_p->value);
      else if (op == Op::Sub)
        return fp(lhs_p->value - rhs_p->value);
      else if (op == Op::Mul)
        return fp(lhs_p->value * rhs_p->value);

      unreachable_code("unknown binary operation!");
    }

    if (lhs_p && lhs_p->value == 0.0) {
      if (op == Op::Add)
        return rhs;
      else if (op == Op::Sub)
        return neg(rhs);
      else if (op == Op::Mul)
        return fp(0.0);
    }

    if (rhs_p && rhs_p->value == 0.0) {
      if (op == Op::Add)
        return lhs;
      else if (op == Op::Sub)
        return lhs;
      else if (op == Op::Mul)
        return fp(0.0);
    }

    if (op == Op::Mul && (lhs_p || rhs_p)) {
      double v = lhs_p ? lhs_p->value : rhs_p->value;
      Expr expr = lhs_p ? rhs : lhs;
      if (v == 1.0)
        return expr;
      else if (v == -1.0)
        return neg(expr);
    }

    return Expr::none();
  }
};

template <class Ret, class Fn>
inline Ret visit(Expr root, const Context &ctx, Fn fn) {
  auto data = ctx.expr_data(root);
  if (const auto *p = data.as_ptr_if<Variable>()) {
    return fn(*p, root);
  } else if (const auto *p = data.as_ptr_if<Fp>()) {
    return fn(*p, root);
  } else if (const auto *p = data.as_ptr_if<Placeholder>()) {
    return fn(*p, root);
  } else if (const auto *p = data.as_ptr_if<SubH>()) {
    return fn(*p, root);
  } else if (const auto *p = data.as_ptr_if<Constraint>()) {
    return fn(*p, root);
  } else if (const auto *p = data.as_ptr_if<Unary>()) {
    return fn(*p, root);
  } else if (const auto *p = data.as_ptr_if<List>()) {
    return fn(*p, root);
  }
  unreachable_code("invalid expression!");
}
} // namespace cxqubo

#endif
