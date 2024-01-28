#include "cxqubo/core/reducer.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
struct TestInserter {
  Context &ctx;
  std::unordered_map<Product, double> poly;
  TestInserter(Context &ctx) : ctx(ctx) {}

  void insert_or_add(ConstSpan<Variable> term, double coeff) {
    Product p = ctx.save_product(term);
    auto [it, inserted] = poly.emplace(p, coeff);
    if (!inserted)
      it->second += coeff;
  }
  bool ignore(ConstSpan<Variable>, double) const { return false; }
};

TEST(reducer_test, basics) {
  Context ctx;
  TestInserter inserter(ctx);
  auto &poly = inserter.poly;
  auto reducer = LimitedInserter(ctx, inserter, 1.0);
  auto x = ctx.create_unnamed_var(Vartype::BINARY);
  auto y = ctx.create_unnamed_var(Vartype::BINARY);
  auto z = ctx.create_unnamed_var(Vartype::BINARY);

  auto xy = ctx.save_product({x, y});
  auto xz = ctx.save_product({x, z});
  auto yz = ctx.save_product({y, z});
  auto z_ = ctx.save_product({z});

  reducer.insert_Hc(z, x, y, 1.0);
  EXPECT_EQ(4, poly.size());
  ASSERT_TRUE(poly.contains(z_));
  ASSERT_TRUE(poly.contains(xy));
  ASSERT_TRUE(poly.contains(xz));
  ASSERT_TRUE(poly.contains(yz));
  EXPECT_EQ(3.0, poly[z_]);
  EXPECT_EQ(1.0, poly[xy]);
  EXPECT_EQ(-2.0, poly[xz]);
  EXPECT_EQ(-2.0, poly[yz]);

  poly.clear();
  reducer.redce_and_insert(xy, 2.0);
  EXPECT_EQ(1, poly.size());
  ASSERT_TRUE(poly.contains(xy));
  EXPECT_EQ(2.0, poly[xy]);

  poly.clear();
  auto xyz = ctx.save_product({x, y, z});
  auto qs = reducer.redce_and_insert(xyz, 1.0);
  ASSERT_EQ(1, qs.size());
  auto q = qs[0];
  auto q_ = ctx.save_product({q});
  auto yq = ctx.save_product({y, q});
  auto zq = ctx.save_product({z, q});
  auto qx = ctx.save_product({q, x});
  EXPECT_EQ(5, poly.size());
  ASSERT_TRUE(poly.contains(zq));
  ASSERT_TRUE(poly.contains(xy));
  ASSERT_TRUE(poly.contains(q_));
  ASSERT_TRUE(poly.contains(yq));
  ASSERT_TRUE(poly.contains(qx));
  EXPECT_EQ(1.0, poly[zq]);
  EXPECT_EQ(3.0, poly[q_]);
  EXPECT_EQ(1.0, poly[xy]);
  EXPECT_EQ(-2.0, poly[yq]);
  EXPECT_EQ(-2.0, poly[qx]);
}
} // namespace
