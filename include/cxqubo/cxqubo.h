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
#include <optional>
#include <sstream>

#include "cimod/binary_quadratic_model.hpp"
#include "cimod/vartypes.hpp"

namespace cxqubo {
inline constexpr double DEFAULT_STRENGTH = 5.0;

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

  friend std::ostream &operator<<(std::ostream &os, const Report &report) {
    os << "energy: " << std::to_string(report.energy) << '\n';
    os << "subhs: " << report.subh_energies << '\n';
    return os << "constraints: " << report.constraint_energies << '\n';
  }
};

/// Class compressing sparse variable indexes to dense indexes.
struct DenseIndexer {
  std::vector<unsigned> *to_sparse = nullptr;
  std::unordered_map<unsigned, unsigned> sparse_to_dense;

public:
  DenseIndexer(std::vector<unsigned> *to_sparse = nullptr)
      : to_sparse(to_sparse) {}

  std::vector<unsigned> indexes(SpanRef<Variable> term) {
    unsigned n = term.size();
    std::vector<unsigned> vec(n, Variable::none().index());
    for (unsigned i = 0; i != n; ++i)
      vec[i] = get_or_assign(term[i].index());
    return vec;
  }

  void reset(std::vector<unsigned> *to_sparse = nullptr) {
    this->to_sparse = to_sparse;
    sparse_to_dense.clear();
  }

  /// Create a sample with sparse indexes from a sample with dense indexes.
  static inline Sample make_sparse(const Sample dense_sample,
                                   const std::vector<unsigned> &to_sparse) {
    Sample sparse_sample;
    for (auto [dense, spin] : dense_sample) {
      auto [it, inserted] = sparse_sample.emplace(to_sparse[dense], spin);
      if (!inserted)
        unreachable_code(
            "to_sparse has a duplicate sparse index for two dense indexes!");
    }
    return sparse_sample;
  }

private:
  unsigned get_or_assign(unsigned sparse) {
    if (!to_sparse)
      return sparse;

    auto it = sparse_to_dense.find(sparse);
    if (it != sparse_to_dense.end())
      return it->second;

    unsigned dense = to_sparse->size();
    to_sparse->emplace_back(sparse);
    sparse_to_dense.emplace(sparse, dense);
    return dense;
  }
};

/// BQM paramter generator.
struct BQMInserter {
  Linear linear;
  Quadratic quad;
  double offset = 0.0;
  DenseIndexer &indexer;

public:
  BQMInserter(DenseIndexer &indexer) : indexer(indexer) {}
  /// Always insert.
  bool ignore(SpanRef<Variable>, double) const { return false; }
  /// Implementation.
  void insert_or_add(SpanRef<Variable> term, double coeff) {
    if (coeff == 0.0)
      return;

    auto indexes = indexer.indexes(term);
    switch (indexes.size()) {
    case 0:
      offset += coeff;
      break;
    case 1: {
      auto [it, inserted] = linear.emplace(indexes[0], coeff);
      if (!inserted)
        it->second += coeff;
    } break;
    case 2: {
      if (indexes[0] == indexes[1]) {
        auto [it, inserted] = linear.emplace(indexes[0], coeff);
        if (!inserted)
          it->second += coeff;
      } else {
        auto [it, inserted] =
            quad.emplace(std::make_pair(indexes[0], indexes[1]), coeff);
        if (!inserted)
          it->second += coeff;
      }
    } break;
    default:
      // TODO: Report error.
      unreachable_code("invalid dimention product!");
    }
  }
};
/// QUBO generator.
struct QUBOInserter {
  Quadratic quad;
  double offset = 0.0;
  DenseIndexer &indexer;

public:
  QUBOInserter(DenseIndexer &indexer) : indexer(indexer) {}
  /// Always insert.
  bool ignore(SpanRef<Variable>, double) const { return false; }
  /// Implementation.
  void insert_or_add(SpanRef<Variable> term, double coeff) {
    if (coeff == 0.0)
      return;

    auto indexes = indexer.indexes(term);
    switch (indexes.size()) {
    case 0:
      offset += coeff;
      break;
    case 1: {
      auto [it, inserted] =
          quad.emplace(std::make_pair(indexes[0], indexes[0]), coeff);
      if (!inserted)
        it->second += coeff;
    } break;
    case 2: {
      auto [it, inserted] =
          quad.emplace(std::make_pair(indexes[0], indexes[1]), coeff);
      if (!inserted)
        it->second += coeff;
    } break;
    default:
      // TODO: Report error.
      unreachable_code("invalid dimention product!");
    }
  }
};

/// Context manager and interface of CXQUBO entities.User generates variables
/// and expressions via CXQUBOModel. All entities constructing a model generated
/// from CXQUBOModel are disposed after lifetime of CXQUBOModel.
///
/// void compute() {
///   auto model = std::make_unique<CXQUBOModel>();
///   auto x = model->add_binary("x");
///   auto y = model->add_binary("y");
///   auto w = model->placeholder("w");
///   auto h = constraint(w * (x + y).pow(2) <= 1.0, "check0");
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

    return Array(&ctx, base, array_shapes.back().as_spanref());
  }

  /// Fix a variable to the given spin value.
  void fix(Express expr, int32_t v) {
    Variable var = ctx.expr_var(expr.ref);
    assert(var && "lhs in 'fix' method must be a variable!");
    Vartype from = ctx.var_data(var).type;
    fixs.emplace(var.index(), convert_spin_value(v, from, Vartype::BINARY));
  }
  /// Fix variables to the given spin value.
  void fix_all(SpanRef<Express> vars, int32_t v) {
    for (const auto &var : vars)
      fix(var, v);
  }
  /// Fix variables to the given spin value.
  void fix_all(Array array, int32_t v) {
    for (auto is : array.array_indexes())
      fix(array.at(is), v);
  }
  /// Fix variables to values.
  void fix_each(SpanRef<Express> vars, SpanRef<int32_t> vals) {
    assert(vars.size() == vals.size() &&
           "number of variables and values must be same!");
    for (unsigned i, n = vars.size(); i != n; ++i)
      fix(vars[i], vals[i]);
  }

public:
  /// Compile an expression represented in AST to polynomial form.
  Compiled compile(Express root) {
    return Compiler(ctx).compile(root.ref, fixs);
  }
  /// Convert a polynomial to cimod's and dimod's BQM parameters. The following
  /// conversions will be applied.
  ///
  /// * Placeholders are replaced to values given in \p feed_dict.
  /// * Terms with dimentions over 2 are reduced to expression 2 or less and \p
  ///   strength is multiplied to the reduced expression as reducing strength.
  /// * Variables' indexes are generally sparse, and they are converted to
  ///   densed ones when \p to_sparse is not nullptr.
  std::tuple<Linear, Quadratic, double>
  create_bqm_params(const Compiled &compiled, std::vector<unsigned> *to_sparse,
                    const FeedDict &feed_dict = FeedDict{},
                    double strength = DEFAULT_STRENGTH) {
    DenseIndexer indexer(to_sparse);
    BQMInserter inserter(indexer);
    create_solver_model(compiled, inserter, feed_dict, strength);
    return std::make_tuple(inserter.linear, inserter.quad, inserter.offset);
  }
  std::tuple<Linear, Quadratic, double>
  create_bqm_params(const Compiled &compiled,
                    const FeedDict &feed_dict = FeedDict{},
                    double strength = DEFAULT_STRENGTH) {
    return create_bqm_params(compiled, nullptr, feed_dict, strength);
  }

  /// Convert a polynomial to cimod::BinaryQuadraticModel.
  BinaryQuadraticModel create_bqm(const Compiled &compiled,
                                  std::vector<unsigned> *to_sparse,
                                  const FeedDict &feed_dict = FeedDict{},
                                  double strength = DEFAULT_STRENGTH) {
    auto [linear, quad, offset] =
        create_bqm_params(compiled, to_sparse, feed_dict, strength);
    return BinaryQuadraticModel(linear, quad, offset,
                                cimod_vartype(Vartype::BINARY));
  }
  BinaryQuadraticModel create_bqm(const Compiled &compiled,
                                  const FeedDict &feed_dict = FeedDict{},
                                  double strength = DEFAULT_STRENGTH) {
    return create_bqm(compiled, nullptr, feed_dict, strength);
  }

  /// Convert a polynomial to QUBO format. See 'create_bqm_params' comment for
  /// details.
  std::tuple<Quadratic, double>
  create_qubo(const Compiled &compiled, std::vector<unsigned> *to_sparse,
              const FeedDict &feed_dict = FeedDict{},
              double strength = DEFAULT_STRENGTH) {
    DenseIndexer indexer(to_sparse);
    QUBOInserter inserter(indexer);
    create_solver_model(compiled, inserter, feed_dict, strength);
    return std::make_tuple(inserter.quad, inserter.offset);
  }
  std::tuple<Quadratic, double>
  create_qubo(const Compiled &compiled, const FeedDict &feed_dict = FeedDict{},
              double strength = DEFAULT_STRENGTH) {
    return create_qubo(compiled, nullptr, feed_dict, strength);
  }

  /// Convert a polynomial to ising format.
  std::tuple<Linear, Quadratic, double>
  create_ising(const Compiled &compiled, std::vector<unsigned> *to_sparse,
               const FeedDict &feed_dict = FeedDict{},
               double strength = DEFAULT_STRENGTH) {
    return create_bqm(compiled, to_sparse, feed_dict, strength).to_ising();
  }
  std::tuple<Linear, Quadratic, double>
  create_ising(const Compiled &compiled, const FeedDict &feed_dict = FeedDict{},
               double strength = DEFAULT_STRENGTH) {
    return create_ising(compiled, nullptr, feed_dict, strength);
  }

  /// Convert a polynomial to an arbitary solver model. If you want to convert
  /// a polynomial to your own model, prepare \p Inserter and pass it as an
  /// argument.
  template <class Inserter>
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
  const Report report(const Compiled &compiled, const Sample &dense_sample,
                      const std::vector<unsigned> &to_sparse,
                      Vartype vartype = Vartype::BINARY,
                      const FeedDict &feed_dict = FeedDict{}) {
    Sample sample = DenseIndexer::make_sparse(dense_sample, to_sparse);
    return report_impl(compiled, sample, vartype, feed_dict);
  }
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
