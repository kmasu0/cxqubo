#include "cxqubo/cxqubo.h"
#include "gtest/gtest.h"

using namespace cxqubo;

namespace {
TEST(cxqubo_test, array) {
  Context context;
  CXQUBOModel model(context);
  Array xs = model.add_vars({3}, Vartype::BINARY, "x");
  EXPECT_EQ("x[0]", context.expr_name((*xs[0]).ref));
  EXPECT_EQ("x[1]", context.expr_name((*xs[1]).ref));
  EXPECT_EQ("x[2]", context.expr_name((*xs[2]).ref));

  xs = model.add_vars({2, 3}, Vartype::BINARY, "x");
  EXPECT_EQ("x[0][0]", context.expr_name((*xs[0][0]).ref));
  EXPECT_EQ("x[0][1]", context.expr_name((*xs[0][1]).ref));
  EXPECT_EQ("x[0][2]", context.expr_name((*xs[0][2]).ref));
  EXPECT_EQ("x[1][0]", context.expr_name((*xs[1][0]).ref));
  EXPECT_EQ("x[1][1]", context.expr_name((*xs[1][1]).ref));
  EXPECT_EQ("x[1][2]", context.expr_name((*xs[1][2]).ref));
}
}
