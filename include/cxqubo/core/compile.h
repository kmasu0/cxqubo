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

#ifndef CXQUBO_CORE_COMPILE_H
#define CXQUBO_CORE_COMPILE_H

#include "cxqubo/core/exprs.h"
#include "cxqubo/core/poly.h"
#include "cxqubo/core/sample.h"
#include "cxqubo/misc/debug.h"

namespace cxqubo {
/// Dictionary of named constants (placeholder) whose value is assigned in
/// evaluation.
using FeedDict = std::unordered_map<std::string_view, double>;

// TODO: Add memory recycler for unordered_map.
class Parser {
  PolyBuilder builder;
  Context &ctx;
  const Sample &fixs;

  unsigned debug_cnt = 0;
  static inline const char *DEBUG_PREFIX = "PARSE: ";

  std::ostream &debug_enter(std::string_view process) {
    return odbg_indent() << "PARSE(" << process << ", " << debug_cnt++
                         << "): >> ";
  }
  std::ostream &debug_exit(std::string_view process) {
    return odbg_indent() << "PARSE(" << process << ", " << --debug_cnt
                         << "): << ";
  }

public:
  Parser(Context &ctx, const Sample &fixs)
      : builder(ctx), ctx(ctx), fixs(fixs) {}

  Poly parse(Expr root) { return visit<Poly>(root, ctx, *this); }
  Poly operator()(Fp v, Expr expr) {
    debug_code(debug_enter("fp") << expr << '\n');
    auto result = builder.constant(expr);
    debug_code(debug_exit("fp") << result << '\n');
    return result;
  }
  Poly operator()(Placeholder v, Expr expr) {
    debug_code(debug_enter("placeholder") << expr << '\n');
    auto result = builder.constant(expr);
    debug_code(debug_exit("placeholder") << result << '\n');
    return result;
  }
  Poly operator()(Variable v, Expr expr) {
    debug_code(debug_enter("variable") << expr << '\n');
    auto it = fixs.find(v.index());
    auto result = it != fixs.end()
                      ? builder.constant(ctx.fp(double(it->second)))
                      : builder.variable(v);
    debug_code(debug_exit("variable") << result << '\n');
    return result;
  }
  Poly operator()(SubH v, Expr expr) {
    debug_code(debug_enter("subh") << expr << '\n');
    auto result = parse(v.expr);
    debug_code(debug_exit("subh") << result << '\n');
    return result;
  }
  Poly operator()(Constraint v, Expr expr) {
    debug_code(debug_enter("constr") << expr << '\n');
    auto result = parse(v.expr);
    debug_code(debug_exit("constr") << result << '\n');
    return result;
  }
  Poly operator()(Unary v, Expr expr) {
    debug_code(debug_enter("unary") << expr << '\n');
    auto result = parse(v.operand);
    builder.neg_assign(result);
    debug_code(debug_exit("unary") << result << '\n');
    return result;
  }
  Poly operator()(List v, Expr expr) {
    debug_code(debug_enter("list") << expr << '\n');
    auto it = v.begin();
    auto result = parse(*it++);
    for (auto end = v.end(); it != end; ++it) {
      auto rhs = parse(*it);
      if (v.op == Op::Add)
        builder.add_assign(result, rhs);
      else if (v.op == Op::Mul)
        builder.mul_assign(result, rhs);
      else
        unreachable_code("unsupported operation");
    }
    debug_code(debug_exit("list") << result << '\n');
    return result;
  }
};

/// Expressions represented in term and coefficient pairs.
struct Compiled {
  Poly poly;
  Expr expr;
};

class Compiler {
  Context &ctx;

public:
  Compiler(Context &ctx) : ctx(ctx) {}

  Compiled compile(Expr root, const Sample &fixs = {}) {
    Parser parser(ctx, fixs);
    return Compiled{parser.parse(root), root};
  }
};

/// Expand placeholders.
struct PlaceholderExpander {
  Context &ctx;
  const FeedDict &feed_dict;

public:
  PlaceholderExpander(Context &ctx, const FeedDict &feed_dict)
      : ctx(ctx), feed_dict(feed_dict) {}

  double expand(Expr root) { return visit<double>(root, ctx, *this); }

  double operator()(Fp data, Expr target) { return data.value; }
  double operator()(Variable data, Expr target) {
    unreachable_code("variable in constant expression is not allowed.");
  }
  double operator()(Placeholder data, Expr target) {
    auto it = feed_dict.find(data.name);
    assert(it != feed_dict.end() && "placeholder does not exist in FeedDict!");
    return it->second;
  }
  double operator()(SubH data, Expr target) { return expand(data.expr); }
  double operator()(Constraint data, Expr target) { return expand(data.expr); }
  double operator()(Unary data, Expr target) {
    assert(data.op == Op::Neg &&
           "unary operator without 'neg' is not supported!");
    return -expand(data.operand);
  }
  double operator()(List data, Expr target) {
    auto it = data.begin();
    double result = expand(*it++);
    for (auto end = data.end(); it != end; ++it) {
      if (data.op == Op::Add)
        result += expand(*it);
      else if (data.op == Op::Mul)
        result *= expand(*it);
      else
        unreachable_code("unsupported operation.");
    }
    return result;
  }
};

struct SubEnergyObserverBase {
  virtual void subh(Expr expr, double energy) {}
  virtual void constraint(Expr expr, double energy) {}
};
using VariableEnergy = std::function<double(Variable)>;
/// Compute energies.
struct ExprEnergy {
  Context &ctx;
  const FeedDict &feed_dict;
  VariableEnergy varenergy;
  std::vector<SubEnergyObserverBase *> observers;

public:
  ExprEnergy(Context &ctx, const FeedDict &feed_dict,
             const VariableEnergy &varenergy)
      : ctx(ctx), feed_dict(feed_dict), varenergy(varenergy) {}

  void add_observer(SubEnergyObserverBase &observer) {
    observers.push_back(&observer);
  }

  double compute(Expr root) { return visit<double>(root, ctx, *this); }

  double operator()(Fp data, Expr target) { return data.value; }
  double operator()(Variable data, Expr target) { return varenergy(data); }
  double operator()(Placeholder data, Expr target) {
    auto it = feed_dict.find(data.name);
    assert(it != feed_dict.end() &&
           "Placeholder which is not registered in FeedDict is found!");
    return it->second;
  }
  double operator()(SubH data, Expr target) {
    double result = compute(data.expr);
    for (auto *observer : observers)
      observer->subh(target, result);
    return result;
  }
  double operator()(Constraint data, Expr target) {
    double result = compute(data.expr);
    for (auto *observer : observers)
      observer->constraint(target, result);
    return result;
  }
  double operator()(Unary data, Expr target) { return -compute(data.operand); }
  double operator()(List data, Expr target) {
    auto it = data.begin();
    double result = compute(*it++);
    for (auto end = data.end(); it != end; ++it) {
      if (data.op == Op::Add)
        result += compute(*it);
      else if (data.op == Op::Mul)
        result *= compute(*it);
      else
        unreachable_code("unsupported operation");
    }
    return result;
  }
};
} // namespace cxqubo

#endif
