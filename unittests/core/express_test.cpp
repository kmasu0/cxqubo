#include "cxqubo/core/express.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
class array_test : public testing::Test {
  std::vector<SpanOwner<unsigned>> shape_owners;

protected:
  void SetUp() override {}
  void TearDown() override { shape_owners.clear(); }

  ArrayShape make_shape(SpanRef<unsigned> vals) {
    shape_owners.push_back(span_owner(vals));
    return shape_owners.back().as_spanref();
  }
};

TEST_F(array_test, basics) {
  Context ctx;
  auto x = ctx.variable(ctx.create_var("x", Vartype::BINARY));
  Array xs(&ctx, x, make_shape({1}));
  EXPECT_EQ(1, xs.size());
  EXPECT_EQ(1, xs.ndim());
  auto e = *xs;
  EXPECT_EQ(x, e.ref);

  auto part = xs[0];
  EXPECT_EQ(1, part.size());
  EXPECT_EQ(0, part.ndim());
  e = *xs;
  EXPECT_EQ(x, e.ref);

  auto y = ctx.create_unnamed_vars(2, Vartype::BINARY);
  auto ey = ctx.variables(y);
  Array ys(&ctx, ey[0], make_shape({2}));
  EXPECT_EQ(2, ys.size());
  EXPECT_EQ(1, ys.ndim());
  e = *ys;
  EXPECT_EQ(ey[0], e.ref);
  e = *ys[1];
  EXPECT_EQ(ey[1], e.ref);

  auto x2d = ctx.create_unnamed_vars(4, Vartype::BINARY);
  auto ex2d = ctx.variables(x2d);
  xs = Array(&ctx, ex2d[0], make_shape({2, 2}));
  EXPECT_EQ(2, xs.size());
  EXPECT_EQ(2, xs.ndim());
  EXPECT_EQ(4, xs.nelements());

  e = *xs;
  EXPECT_EQ(ex2d[0], e.ref);

  part = xs[1];
  EXPECT_EQ(2, part.size());
  EXPECT_EQ(1, part.ndim());
  EXPECT_EQ(ex2d[2], (*part[0]).ref);
  EXPECT_EQ(ex2d[3], (*part[1]).ref);

  EXPECT_EQ(ex2d[0], (*xs[0][0]).ref);
  EXPECT_EQ(ex2d[1], (*xs[0][1]).ref);
  EXPECT_EQ(ex2d[2], (*xs[1][0]).ref);
  EXPECT_EQ(ex2d[3], (*xs[1][1]).ref);
}
} // namespace
