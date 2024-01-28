#include "cxqubo/core/context.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
TEST(variables_test, basics) {
  Context ctx;
  auto v0 = ctx.create_var("var", Vartype::SPIN);
  auto data = ctx.var_data(v0);
  EXPECT_EQ(0, v0.index());
  EXPECT_EQ("var", data.name);
  EXPECT_EQ(Vartype::SPIN, data.type);

  auto v = ctx.create_unnamed_var(Vartype::BINARY);
  data = ctx.var_data(v);
  EXPECT_EQ(1, v.index());
  EXPECT_EQ("", data.name);
  EXPECT_EQ(Vartype::BINARY, data.type);

  v = ctx.create_var("", Vartype::BINARY);
  EXPECT_EQ(2, v.index());
  data = ctx.var_data(v);
  EXPECT_EQ("", data.name);
  EXPECT_EQ(Vartype::BINARY, data.type);

  auto vs = ctx.create_unnamed_vars(3, Vartype::BINARY);
  ASSERT_EQ(3, vs.size());
  EXPECT_EQ(3, vs[0].index());
  EXPECT_EQ(4, vs[1].index());
  EXPECT_EQ(5, vs[2].index());
  for (auto v : vs) {
    data = ctx.var_data(v);
    EXPECT_EQ("", data.name);
    EXPECT_EQ(Vartype::BINARY, data.type);
  }

  auto v1 = ctx.var_of("var");
  EXPECT_EQ(v0, v1);
}

TEST(exprs_test, basics) {
  Context ctx;

  auto e = ctx.fp(1.2);
  EXPECT_EQ(0, e.index());
  ExprData data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(1.2), data.as<Fp>());

  e = ctx.variable(Variable::from(0));
  EXPECT_EQ(1, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<Variable>());
  EXPECT_EQ(Variable::from(0), data.as<Variable>());

  e = ctx.placeholder("p");
  EXPECT_EQ(2, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<Placeholder>());
  EXPECT_EQ(make<Placeholder>("p"), data.as<Placeholder>());

  e = ctx.subh("subh", Expr::from(3));
  EXPECT_EQ(3, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<SubH>());
  EXPECT_EQ(make<SubH>("subh", Expr::from(3)), data.as<SubH>());

  e = ctx.constraint("constr", Expr::from(4), Condition::from(0));
  EXPECT_EQ(4, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<Constraint>());
  EXPECT_EQ(make<Constraint>("constr", Expr::from(4), Condition::from(0)),
            data.as<Constraint>());

  e = ctx.neg(Expr::from(1));
  EXPECT_EQ(5, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<Unary>());
  EXPECT_EQ(make<Unary>(Op::Neg, Expr::from(1)), data.as<Unary>());

  e = ctx.add(Expr::from(1), Expr::from(3));
  EXPECT_EQ(6, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<List>());
  auto add = data.as<List>();
  EXPECT_EQ(Op::Add, add.op);
  EXPECT_EQ(Expr::from(1), add.node->value);
  EXPECT_NE(nullptr, add.node->next);
  EXPECT_EQ(Expr::from(3), add.node->next->value);

  e = ctx.mul(Expr::from(1), Expr::from(3));
  EXPECT_EQ(7, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<List>());
  auto mul = data.as<List>();
  EXPECT_EQ(Op::Mul, mul.op);
  EXPECT_EQ(Expr::from(1), mul.node->value);
  EXPECT_NE(nullptr, mul.node->next);
  EXPECT_EQ(Expr::from(3), mul.node->next->value);

  e = ctx.sub(Expr::from(1), Expr::from(3));
  EXPECT_EQ(9, e.index());
  data = ctx.expr_data(e);
  ASSERT_TRUE(data.is<List>());
  auto sub = data.as<List>();
  EXPECT_EQ(Op::Add, sub.op);
  EXPECT_EQ(Expr::from(1), sub.node->value);
  EXPECT_NE(nullptr, sub.node->next);
  data = ctx.expr_data(sub.node->next->value);
  EXPECT_TRUE(data.is<Unary>());
  EXPECT_EQ(make<Unary>(Op::Neg, Expr::from(3)), data.as<Unary>());
}

TEST(exprs_test, binop_list) {
  Context ctx;
  auto e0 = ctx.variable(Variable::from(0));
  auto e1 = ctx.variable(Variable::from(1));
  auto e2 = ctx.variable(Variable::from(2));
  auto e3 = ctx.variable(Variable::from(3));

  auto lhs = ctx.add(e0, e1);
  List data = ctx.expr_data(lhs).as<List>();
  EXPECT_EQ(Op::Add, data.op);
  auto it = data.begin();
  EXPECT_EQ(e0, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e1, *it);
  ASSERT_EQ(nullptr, it.next());

  auto rhs = ctx.add(e2, e3);
  data = ctx.expr_data(rhs).as<List>();
  EXPECT_EQ(Op::Add, data.op);
  it = data.begin();
  EXPECT_EQ(e2, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e3, *it);
  ASSERT_EQ(nullptr, it.next());

  auto e = ctx.add(lhs, rhs);
  data = ctx.expr_data(e).as<List>();
  EXPECT_EQ(Op::Add, data.op);

  it = data.begin();
  EXPECT_EQ(lhs, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e2, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e3, *it);
  EXPECT_EQ(nullptr, it.next());

  e = ctx.add(lhs, e3);
  data = ctx.expr_data(e).as<List>();
  it = data.begin();
  EXPECT_EQ(e3, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e0, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e1, *it);
  EXPECT_EQ(nullptr, it.next());

  e = ctx.add(e0, rhs);
  data = ctx.expr_data(e).as<List>();
  it = data.begin();
  EXPECT_EQ(e0, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e2, *it);
  ASSERT_NE(nullptr, it.next());
  ++it;
  EXPECT_EQ(e3, *it);
  EXPECT_EQ(nullptr, it.next());
}

TEST(exprs_test, constfold) {
  Context ctx;

  auto f0 = ctx.fp(0.0);
  auto f1 = ctx.fp(1.0);
  auto f2 = ctx.fp(2.0);
  auto f3 = ctx.fp(3.0);
  auto fminus = ctx.fp(-1.0);
  auto v0 = ctx.variable(Variable::from(0));

  auto data = ctx.expr_data(ctx.neg(f2));
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(-2.0), data.as<Fp>());

  data = ctx.expr_data(ctx.add(f2, f3));
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(5.0), data.as<Fp>());
  EXPECT_EQ(v0, ctx.add(f0, v0));
  EXPECT_EQ(v0, ctx.add(v0, f0));

  data = ctx.expr_data(ctx.sub(f1, f3));
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(-2.0), data.as<Fp>());

  data = ctx.expr_data(ctx.sub(f0, v0));
  ASSERT_TRUE(data.is<Unary>());
  EXPECT_EQ(make<Unary>(Op::Neg, v0), data.as<Unary>());
  EXPECT_EQ(v0, ctx.sub(v0, f0));

  auto mul = ctx.mul(f2, f3);
  data = ctx.expr_data(mul);
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(6.0), data.as<Fp>());
  data = ctx.expr_data(ctx.mul(f0, v0));
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(0.0), data.as<Fp>());
  data = ctx.expr_data(ctx.mul(v0, f0));
  ASSERT_TRUE(data.is<Fp>());
  EXPECT_EQ(make<Fp>(0.0), data.as<Fp>());

  data = ctx.expr_data(ctx.mul(f1, v0));
  ASSERT_TRUE(data.is<Variable>());
  EXPECT_EQ(v0, data.as<Variable>());
  data = ctx.expr_data(ctx.mul(v0, f1));
  ASSERT_TRUE(data.is<Variable>());
  EXPECT_EQ(v0, data.as<Variable>());

  data = ctx.expr_data(ctx.mul(fminus, v0));
  ASSERT_TRUE(data.is<Unary>());
  EXPECT_EQ(make<Unary>(Op::Neg, v0), data.as<Unary>());
  data = ctx.expr_data(ctx.mul(v0, fminus));
  ASSERT_TRUE(data.is<Unary>());
  EXPECT_EQ(make<Unary>(Op::Neg, v0), data.as<Unary>());
}

TEST(products_test, basics) {
  Context ctx;
  auto v0 = Variable::from(0);
  auto v1 = Variable::from(1);
  auto v2 = Variable::from(2);

  auto p = ctx.save_product({});
  EXPECT_EQ(Product::none(), p);
  EXPECT_EQ(0, ctx.dim_of(p));

  p = ctx.save_product(v0, true);
  EXPECT_EQ(Product::from(0), p);
  auto data = ctx.product_data(p);
  ASSERT_EQ(1, data.size());
  EXPECT_EQ(v0, data[0]);

  p = ctx.save_product({v0, v1}, true);
  EXPECT_EQ(Product::from(1), p);
  data = ctx.product_data(p);
  ASSERT_EQ(2, data.size());
  EXPECT_EQ(v0, data[0]);
  EXPECT_EQ(v1, data[1]);

  p = ctx.save_product({v1, v0}, true);
  EXPECT_EQ(Product::from(2), p);
  data = ctx.product_data(p);
  ASSERT_EQ(2, data.size());
  EXPECT_EQ(v1, data[0]);
  EXPECT_EQ(v0, data[1]);

  p = ctx.save_product({v1, v0});
  EXPECT_EQ(Product::from(1), p);

  auto p2 = ctx.save_product({v1, v2});
  auto mul = ctx.mul_products(p, p2);
  data = ctx.product_data(mul);
  ASSERT_EQ(4, data.size());
  EXPECT_EQ(v0, data[0]);
  EXPECT_EQ(v1, data[1]);
  EXPECT_EQ(v1, data[2]);
  EXPECT_EQ(v2, data[3]);
}

TEST(context_test, convert_sample) {
  Context ctx;
  auto s0 = ctx.create_unnamed_var(Vartype::SPIN);
  auto s1 = ctx.create_unnamed_var(Vartype::SPIN);
  auto b2 = ctx.create_unnamed_var(Vartype::BINARY);
  auto b3 = ctx.create_unnamed_var(Vartype::BINARY);

  auto sample = Sample{
      {s0.index(), 0},
      {s1.index(), 1},
      {b2.index(), 0},
      {b3.index(), 1},
  };
  auto converted = ctx.convert_sample(sample, Vartype::BINARY);
  ASSERT_EQ(4, converted.size());
  EXPECT_EQ(-1, converted[s0.index()]);
  EXPECT_EQ(1, converted[s1.index()]);
  EXPECT_EQ(0, converted[b2.index()]);
  EXPECT_EQ(1, converted[b3.index()]);

  sample = Sample{
      {s0.index(), -1},
      {s1.index(), 1},
      {b2.index(), -1},
      {b3.index(), 1},
  };
  converted = ctx.convert_sample(sample, Vartype::SPIN);
  EXPECT_EQ(-1, converted[s0.index()]);
  EXPECT_EQ(1, converted[s1.index()]);
  EXPECT_EQ(0, converted[b2.index()]);
  EXPECT_EQ(1, converted[b3.index()]);
}

TEST(conditions_test, basics) {
  Context ctx;
  EXPECT_EQ(1, ctx.num_cmps());
  EXPECT_TRUE(ctx.contains_cmp(CmpOp::eq(), 0.0));

  auto eqz = ctx.eqz();
  EXPECT_EQ(0, eqz.index());
  EXPECT_EQ(0.0 == 0.0, ctx.apply_cond(eqz, 0.0));
  EXPECT_EQ(1.0 == 0.0, ctx.apply_cond(eqz, 1.0));

  auto ge = ctx.insert_cmp(CmpOp::ge(), 2.0);
  EXPECT_EQ(1, ge.index());
  EXPECT_EQ(2, ctx.num_cmps());
  EXPECT_TRUE(ctx.contains_cmp(CmpOp::ge(), 2.0));

  EXPECT_EQ(1.0 >= 2.0, ctx.apply_cond(ge, 1.0));
  EXPECT_EQ(2.0 >= 2.0, ctx.apply_cond(ge, 2.0));
  EXPECT_EQ(3.0 >= 2.0, ctx.apply_cond(ge, 3.0));

  auto lt = ctx.insert_cmp(CmpOp::lt(), 2.0);
  EXPECT_EQ(2, lt.index());
  EXPECT_EQ(3, ctx.num_cmps());
  EXPECT_TRUE(ctx.contains_cmp(CmpOp::lt(), 2.0));

  EXPECT_EQ(1.0 < 2.0, ctx.apply_cond(lt, 1.0));
  EXPECT_EQ(2.0 < 2.0, ctx.apply_cond(lt, 2.0));
  EXPECT_EQ(3.0 < 2.0, ctx.apply_cond(lt, 3.0));
}
} // namespace
