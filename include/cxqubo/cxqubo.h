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

#ifndef CXQUBO_CXQUBO_H
#define CXQUBO_CXQUBO_H

#include "cxqubo/core/compile.h"
#include "cxqubo/core/express.h"
#include "cxqubo/core/reducer.h"
#include "cxqubo/misc/drawable.h"
#include "cxqubo/misc/strsaver.h"
#include <sstream>

#include "cimod/binary_quadratic_model.hpp"
#include "cimod/vartypes.hpp"

namespace cxqubo {
using Linear = cimod::Linear<unsigned, double>;
using Quadratic = cimod::Quadratic<unsigned, double>;

using DecodedLinear = cimod::Linear<std::string_view, double>;
using DecodedQuadratic = cimod::Quadratic<std::string_view, double>;

using BinaryQuadraticModel =
    cimod::BinaryQuadraticModel<unsigned, double, cimod::Dense>;

inline cimod::Vartype cimod_vartype(Vartype vartype) {
  return cimod::Vartype(vartype);
}

template <class K, class V, class H>
inline std::vector<std::pair<K, V>>
sorted(const std::unordered_map<K, V, H> &m) {
  std::vector<std::pair<K, V>> vec(m.begin(), m.end());
  std::sort(vec.begin(), vec.end(),
            [](const std::pair<K, V> &lhs, const std::pair<K, V> &rhs) {
              return lhs.first < rhs.first;
            });
  return vec;
}

/// A readable sampling report.
struct Report {
  const Context *context;
  Vartype vartype;

  DecodedSample sample;
  DecodedSample fixed;

  double energy;
  std::unordered_map<std::string_view, double> subh_energies;
  std::unordered_map<std::string_view, std::pair<bool, double>>
      constraint_energies;

public:
  /// SubH labels and energies.
  const std::unordered_map<std::string_view, double> &subhs() const {
    return subh_energies;
  }

  /// Constraint labels and energies.
  std::unordered_map<std::string_view, std::pair<bool, double>>
  constraints(bool only_broken = true) const {
    if (!only_broken)
      return constraint_energies;

    std::unordered_map<std::string_view, std::pair<bool, double>> result;
    for (auto [label, broken_energy] : constraint_energies)
      if (broken_energy.first)
        result.emplace(label, broken_energy);
    return result;
  }

  /// Return a spin value of the given variable.
  std::optional<int32_t> spin(Express variable) const {
    auto var = context->expr_var(variable.ref);
    return spin(var);
  }
  /// Return a spin value of the given variable.
  std::optional<int32_t> spin(Variable var) const {
    return var ? spin(context->var_data(var).name) : std::nullopt;
  }
  /// Return a spin value of the given name.
  std::optional<int32_t> spin(std::string_view name) const {
    auto it = sample.find(name);
    if (it != sample.end())
      return it->second;

    it = fixed.find(name);
    if (it != fixed.end())
      return it->second;

    return std::nullopt;
  }

  std::ostream &draw(std::ostream &os) const {
    os << "energy: " << std::to_string(energy) << '\n';
    os << "subhs: " << subh_energies << '\n';
    return os << "constraints: " << constraint_energies << '\n';
  }
};

/// BQM paramter generator.
struct BQMInserter {
  Linear linear;
  Quadratic quad;
  double offset = 0.0;

public:
  BQMInserter() = default;
  void insert_or_add(ConstSpan<Variable> term, double coeff) {
    if (coeff == 0.0)
      return;

    switch (term.size()) {
    case 0:
      offset += coeff;
      break;
    case 1: {
      auto [it, inserted] = linear.emplace(term[0].index(), coeff);
      if (!inserted)
        it->second += coeff;
    } break;
    case 2: {
      if (term[0] == term[1]) {
        auto [it, inserted] = linear.emplace(term[0].index(), coeff);
        if (!inserted)
          it->second += coeff;
      } else {
        auto [it, inserted] = quad.emplace(
            std::make_pair(term[0].index(), term[1].index()), coeff);
        if (!inserted)
          it->second += coeff;
      }
    } break;
    default:
      // TODO: Report error.
      unreachable_code("invalid dimention product!");
    }
  }
  bool ignore(ConstSpan<Variable>, double) const { return false; }
};
/// QUBO generator.
struct QUBOInserter {
  Quadratic quad;
  double offset = 0.0;

public:
  QUBOInserter() = default;
  void insert_or_add(ConstSpan<Variable> term, double coeff) {
    if (coeff == 0.0)
      return;

    switch (term.size()) {
    case 0:
      offset += coeff;
      break;
    case 1: {
      auto [it, inserted] =
          quad.emplace(std::make_pair(term[0].index(), term[0].index()), coeff);
      if (!inserted)
        it->second += coeff;
    } break;
    case 2: {
      auto [it, inserted] =
          quad.emplace(std::make_pair(term[0].index(), term[1].index()), coeff);
      if (!inserted)
        it->second += coeff;
    } break;
    default:
      // TODO: Report error.
      unreachable_code("invalid dimention product!");
    }
  }
  bool ignore(ConstSpan<Variable>, double) const { return false; }
};

/// Context manager and interface of CXQUBO entities.User generates variables
/// and expressions via CXQUBOModel. All entities constructing a model generated
/// from CXQUBOModel are disposed after lifetime of CXQUBOModel.
///
/// void compute() {
///   auto model = std::make_unique<CXQUBOModel>();
///   auto x = model->create_binary("x");
///   auto y = model->create_binary("y");
///   auto w = model->placeholder("w");
///   auto h = w * (x + y).pow(2);
///   h = model->constraint(h <= 1.0, "check0");
///   auto result = model.compile(h);
///   auto [qubo, offset] = model.create_qubo(result, {"w": 3.1});
///   std::cout << qubo << '\n';
///   ...
///   // 'x', 'y', 'w', 'h', 'result' and 'qubo' are disposed after 'model' is
///   // distructed.
/// }
class CXQUBOModel {
private:
  /// Storage of entities.
  Context &ctx;
  /// Array data.
  std::vector<SpanOwner<unsigned>> array_shapes;
  /// Fixed variables' values.
  Sample fixs;

public:
  CXQUBOModel(Context &ctx) : ctx(ctx) {}
  CXQUBOModel(const CXQUBOModel &) = delete;
  CXQUBOModel &operator=(const CXQUBOModel &) = delete;

  std::string_view decode(unsigned id) const {
    return ctx.var_data(Variable::from(id)).name;
  }

public:
  Context &context() { return ctx; }

  /// Return a variable name if it exists. Otherwise create a variable name and
  /// return it.
  std::string_view decode_or_create_name(unsigned id) {
    auto result = decode(id);
    if (result.empty()) {
      std::stringstream ss;
      ss << "<unnamed>." << Variable::from(id);
      result = ctx.save_string(ss.str());
    }
    return result;
  }

  std::unordered_map<std::string, std::string>
  decode(const Compiled &compiled) {
    std::unordered_map<std::string, std::string> result;
    for (auto [term, coeff] : compiled.poly) {
      std::stringstream term_ss;
      std::stringstream coeff_ss;
      ctx.draw_product(term_ss, term);
      ctx.draw_expr(coeff_ss, coeff);
      result.emplace(term_ss.str(), coeff_ss.str());
    }
    return result;
  }

  /// Convert Linear to readable data.
  DecodedLinear decode(const Linear &linear) {
    DecodedLinear result;
    for (auto [id, coeff] : linear) {
      auto name = decode_or_create_name(id);
      assert(!result.contains(name) &&
             "Different product for same variables is generated!");
      result.emplace(name, coeff);
    }
    return result;
  }
  /// Convert Quadratic to readable data.
  DecodedQuadratic decode(const Quadratic &quad) {
    DecodedQuadratic result;
    for (auto [term, coeff] : quad) {
      auto v0 = decode_or_create_name(term.first);
      auto v1 = decode_or_create_name(term.second);
      auto v0v1 = std::make_pair(v0, v1);
      assert(!result.contains(v0v1) &&
             "Different product for same variables is generated!");
      result.emplace(v0v1, coeff);
    }
    return result;
  }
  /// Convert Sample to readable data.
  DecodedSample decode(const Sample &sample) {
    DecodedSample result;
    for (auto [id, value] : sample)
      result.emplace(decode_or_create_name(id), value);

    return result;
  }

  /// Return a floating point value.
  Express fp(double value) { return Express(&ctx, ctx.fp(value)); }
  /// Return a placeholder (named constant).
  Express placeholder(std::string_view name) {
    return Express(&ctx, ctx.placeholder(name));
  }

  /// Create and return a variable.
  Express add_var(Vartype vartype, std::string_view name = "") {
    return Express(&ctx, ctx.variable(ctx.create_var(name, vartype)));
  }
  /// Create and return a binary variable.
  Express add_binary(std::string_view name = "") {
    return add_var(Vartype::BINARY, name);
  }
  /// Create and return a spin variable.
  Express add_spin(std::string_view name = "") {
    return add_var(Vartype::SPIN, name);
  }
  /// Return an array which includes several variables.
  Array add_vars(ArrayShape shape, Vartype type,
                 std::string_view basename = "") {
    Expr base;
    for (auto indexes : shape_range(shape)) {
      std::string_view name = basename;
      if (!name.empty()) {
        std::stringstream ss;
        ss << name;
        for (size_t i : indexes)
          ss << '[' << i << ']';

        name = ctx.save_string(ss.str());
      }

      auto var = ctx.create_var(name, type);
      Expr expr = ctx.variable(var);
      if (!base)
        base = expr;
    }

    array_shapes.push_back(span_owner(shape));

    return Array(&ctx, base, array_shapes.back().as_const_span());
  }

  /// Fix a variable to the given spin value.
  void fix(Express expr, int32_t v) {
    Variable var = ctx.expr_var(expr.ref);
    assert(var && "lhs in 'fix' method must be a variable!");
    Vartype from = ctx.var_data(var).type;
    fixs.emplace(var.index(), convert_spin_value(v, from, Vartype::BINARY));
  }
  /// Fix variables to the given spin value.
  void fix_all(ConstSpan<Express> vars, int32_t v) {
    for (const auto &var : vars)
      fix(var, v);
  }
  /// Fix variables to the given spin value.
  void fix_all(Array array, int32_t v) {
    for (auto is : array.array_indexes())
      fix(array.at(is), v);
  }
  /// Fix variables to values.
  void fix_each(ConstSpan<Express> vars, ConstSpan<int32_t> vals) {
    assert(vars.size() == vals.size() &&
           "number of variables and values must be same!");
    for (unsigned i, n = vars.size(); i != n; ++i)
      fix(vars[i], vals[i]);
  }

public:
  /// Convert a AST to polynomials.
  Compiled compile(Express root) {
    return Compiler(ctx).compile(root.ref, fixs);
  }
  /// Convert a Compiled to cimod's and dimod's BQM parameters.
  std::tuple<Linear, Quadratic, double>
  create_bqm_params(const Compiled &compiled,
                    const FeedDict &feed_dict = FeedDict{},
                    double strength = DEFAULT_STRENGTH) {
    BQMInserter inserter;
    create_solver_model(compiled, inserter, feed_dict, strength);
    return std::make_tuple(inserter.linear, inserter.quad, inserter.offset);
  }
  /// Convert a Compiled to cimod::BinaryQuadraticModel.
  BinaryQuadraticModel create_bqm(const Compiled &compiled,
                                  const FeedDict &feed_dict = FeedDict{},
                                  double strength = DEFAULT_STRENGTH) {
    auto [linear, quad, offset] =
        create_bqm_params(compiled, feed_dict, strength);
    return BinaryQuadraticModel(linear, quad, offset,
                                cimod_vartype(Vartype::BINARY));
  }
  /// Convert a Compiled to QUBO format.
  std::tuple<Quadratic, double>
  create_qubo(const Compiled &compiled, const FeedDict &feed_dict = FeedDict{},
              double strength = DEFAULT_STRENGTH) {
    QUBOInserter inserter;
    create_solver_model(compiled, inserter, feed_dict, strength);
    return std::make_tuple(inserter.quad, inserter.offset);
  }
  /// Convert a Compiled to ising format.
  std::tuple<Linear, Quadratic, double>
  create_ising(const Compiled &compiled, const FeedDict &feed_dict = FeedDict{},
               double strength = DEFAULT_STRENGTH) {
    return create_bqm(compiled, feed_dict, strength).to_ising();
  }
  /// Convert a Compiled to an arbitary solver model. If you want to convert
  /// Compiled to your own model, prepare \p TermCoeffInserter and pass it as an
  /// argument.
  template <TermCoeffInserter Inserter>
  void create_solver_model(const Compiled &compiled, Inserter &inserter,
                           const FeedDict &feed_dict = FeedDict{},
                           double strength = DEFAULT_STRENGTH) {
    // TODO: Throw exception.
    assert(!compiled.poly.empty() &&
           "Polynomial has not been created. Call 'compile()' method.");

    PlaceholderExpander expander(ctx, feed_dict);
    auto reducer = LimitedInserter(ctx, inserter, strength);

    for (auto [term, coeff_expr] : compiled.poly) {
      double coeff = expander.expand(coeff_expr);
      reducer.redce_and_insert(term, coeff);
    }
  }
  /// Return readable sampling result.
  const Report report(const Compiled &compiled, const Sample &sample,
                      Vartype vartype = Vartype::BINARY,
                      const FeedDict &feed_dict = FeedDict{}) {
    return report_impl(compiled, sample, vartype, feed_dict);
  }

private:
  struct SubEnergyReporter : public SubEnergyObserverBase {
    Report &r;
    const Context &ctx;

    SubEnergyReporter(Report &report, const Context &ctx)
        : r(report), ctx(ctx) {}

    void subh(Expr expr, double energy) override {
      r.subh_energies.emplace(ctx.expr_name(expr), energy);
    }
    void constraint(Expr expr, double energy) override {
      auto data = ctx.expr_data(expr);
      auto constr = data.as<Constraint>();
      bool is_broken = ctx.apply_cond(constr.cond, energy);
      r.constraint_energies.emplace(constr.label,
                                    std::make_pair(is_broken, energy));
    }
  };
  const Report report_impl(const Compiled &compiled, const Sample &sample,
                           Vartype vartype, const FeedDict &feed_dict) {
    Report r;
    r.context = &ctx;
    r.vartype = vartype;
    r.sample = decode(ctx.convert_sample(sample, vartype));
    r.fixed = decode(ctx.convert_sample(fixs, Vartype::BINARY));

    ExprEnergy ee(ctx, feed_dict,
                  [this, &sample, vartype](Variable var) -> double {
                    Vartype to = ctx.var_data(var).type;
                    auto it = sample.find(var.index());
                    if (it != sample.end()) {
                      return convert_spin_value(it->second, vartype, to);
                    }
                    it = fixs.find(var.index());
                    assert(it != fixs.end() &&
                           "unknown variable is found in sampling result!");
                    return it->second;
                  });
    SubEnergyReporter reporter(r, ctx);
    ee.add_observer(reporter);
    r.energy = ee.compute(compiled.expr);

    return r;
  }
};
} // namespace cxqubo

#endif
