#include "cxqubo/core/compile.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
TEST(parser_test, basics) {
  Context ctx;
  Sample fixs;
  auto b0 = ctx.create_unnamed_var(Vartype::BINARY);
  auto b1 = ctx.create_unnamed_var(Vartype::BINARY);
  auto b2 = ctx.create_unnamed_var(Vartype::BINARY);
  auto b3 = ctx.create_unnamed_var(Vartype::BINARY);

  auto e0 = ctx.variable(b0);
  auto e1 = ctx.variable(b1);
  auto e2 = ctx.variable(b2);
  auto e3 = ctx.variable(b3);

  Parser parser(ctx, fixs);
  auto poly = parser.parse(ctx.fp(1.2));
  EXPECT_TRUE(poly.is<Single>());
  auto single = poly.as<Single>();
  EXPECT_EQ(Product::none(), single.first);
  EXPECT_EQ(ctx.fp(1.2), single.second);

  poly = parser.parse(ctx.placeholder("v"));
  EXPECT_TRUE(poly.is<Single>());
  single = poly.as<Single>();
  EXPECT_EQ(Product::none(), single.first);
  EXPECT_EQ(ctx.placeholder("v"), single.second);

  poly = parser.parse(e0);
  EXPECT_TRUE(poly.is<Single>());
  single = poly.as<Single>();
  EXPECT_EQ(ctx.save_product(b0), single.first);
  EXPECT_EQ(ctx.fp(1.0), single.second);

  poly = parser.parse(ctx.subh("subh", e1));
  EXPECT_TRUE(poly.is<Single>());
  single = poly.as<Single>();
  EXPECT_EQ(ctx.save_product(b1), single.first);
  EXPECT_EQ(ctx.fp(1.0), single.second);

  poly = parser.parse(ctx.constraint("constr", e2, Condition::from(0)));
  EXPECT_TRUE(poly.is<Single>());
  single = poly.as<Single>();
  EXPECT_EQ(ctx.save_product(b2), single.first);
  EXPECT_EQ(ctx.fp(1.0), single.second);

  poly = parser.parse(ctx.neg(e3));
  EXPECT_TRUE(poly.is<Single>());
  single = poly.as<Single>();
  EXPECT_EQ(ctx.save_product(b3), single.first);
  EXPECT_EQ(ctx.fp(-1.0), single.second);

  poly = parser.parse(ctx.add(e0, e1));
  EXPECT_TRUE(poly.is<Multi>());
  auto multi = poly.as<Multi>();
  EXPECT_EQ(2, multi.size());

  auto p0 = ctx.save_product(b0);
  auto p1 = ctx.save_product(b1);
  EXPECT_TRUE(multi.contains(p0));
  EXPECT_TRUE(multi.contains(p1));
  EXPECT_EQ(ctx.fp(1.0), multi[p0]);
  EXPECT_EQ(ctx.fp(1.0), multi[p1]);

  poly = parser.parse(ctx.mul(e0, e1));
  EXPECT_TRUE(poly.is<Single>());
  single = poly.as<Single>();
  p0 = ctx.save_product({b0, b1});
  EXPECT_EQ(ctx.save_product({b0, b1}), single.first);
  EXPECT_EQ(ctx.fp(1.0), single.second);

  poly = parser.parse(ctx.mul(ctx.add(e0, e1), ctx.add(e2, e3)));
  EXPECT_TRUE(poly.is<Multi>());
  multi = poly.as<Multi>();
  EXPECT_EQ(4, multi.size());

  p0 = ctx.save_product({b0, b2});
  p1 = ctx.save_product({b0, b3});
  auto p2 = ctx.save_product({b1, b2});
  auto p3 = ctx.save_product({b1, b3});
  EXPECT_TRUE(multi.contains(p0));
  EXPECT_TRUE(multi.contains(p1));
  EXPECT_TRUE(multi.contains(p2));
  EXPECT_TRUE(multi.contains(p3));
  EXPECT_EQ(ctx.fp(1.0), multi[p0]);
  EXPECT_EQ(ctx.fp(1.0), multi[p1]);
  EXPECT_EQ(ctx.fp(1.0), multi[p2]);
  EXPECT_EQ(ctx.fp(1.0), multi[p3]);

  poly = parser.parse(ctx.neg(ctx.add(e0, e1)));
  EXPECT_TRUE(poly.is<Multi>());
  multi = poly.as<Multi>();
  EXPECT_EQ(2, multi.size());
  p0 = ctx.save_product(b0);
  p1 = ctx.save_product(b1);
  EXPECT_TRUE(multi.contains(p0));
  EXPECT_TRUE(multi.contains(p1));
  EXPECT_EQ(ctx.fp(-1.0), multi[p0]);
  EXPECT_EQ(ctx.fp(-1.0), multi[p1]);

  auto s0 = ctx.create_unnamed_var(Vartype::SPIN);
  poly = parser.parse(ctx.variable(s0));
  EXPECT_TRUE(poly.is<Multi>());
  multi = poly.as<Multi>();
  EXPECT_EQ(2, multi.size());
  EXPECT_TRUE(multi.contains(Product::none()));
  EXPECT_EQ(ctx.fp(-1.0), multi[Product::none()]);
  p0 = ctx.save_product(s0);
  EXPECT_TRUE(multi.contains(p0));
  EXPECT_EQ(ctx.fp(2.0), multi[p0]);
}

TEST(parser_test, fixs) {
  Context ctx;
  auto s0 = ctx.create_unnamed_var(Vartype::SPIN);
  Sample fixs({{s0.index(), -1}});
  Parser parser(ctx, fixs);
  auto poly = parser.parse(ctx.variable(s0));
  EXPECT_TRUE(poly.is<Single>());
  auto single = poly.as<Single>();
  EXPECT_EQ(Product::none(), single.first);
  EXPECT_EQ(ctx.fp(-1.0), single.second);
}
} // namespace
