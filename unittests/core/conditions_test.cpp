#include "cxqubo/core/conditions.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
TEST(conditions_test, cmpop_basics) {
  EXPECT_EQ(0, CmpOp::INVALID);
  EXPECT_EQ(CmpOp::EQ, CmpOp::eq().kind);
  EXPECT_EQ(CmpOp::NE, CmpOp::ne().kind);
  EXPECT_EQ(CmpOp::GT, CmpOp::gt().kind);
  EXPECT_EQ(CmpOp::GE, CmpOp::ge().kind);
  EXPECT_EQ(CmpOp::LT, CmpOp::lt().kind);
  EXPECT_EQ(CmpOp::LE, CmpOp::le().kind);

  EXPECT_EQ(-2 == 2, CmpOp::eq().invoke(-2, 2));
  EXPECT_EQ(2 == 2, CmpOp::eq().invoke(2, 2));

  EXPECT_EQ(-2 != 2, CmpOp::ne().invoke(-2, 2));
  EXPECT_EQ(2 != 2, CmpOp::ne().invoke(2, 2));

  EXPECT_EQ(-2 > 2, CmpOp::gt().invoke(-2, 2));
  EXPECT_EQ(2 > 2, CmpOp::gt().invoke(2, 2));
  EXPECT_EQ(2 > -2, CmpOp::gt().invoke(2, -2));

  EXPECT_EQ(-2 >= 2, CmpOp::ge().invoke(-2, 2));
  EXPECT_EQ(2 >= 2, CmpOp::ge().invoke(2, 2));
  EXPECT_EQ(2 >= -2, CmpOp::ge().invoke(2, -2));

  EXPECT_EQ(-2 < 2, CmpOp::lt().invoke(-2, 2));
  EXPECT_EQ(2 < 2, CmpOp::lt().invoke(2, 2));
  EXPECT_EQ(2 < -2, CmpOp::lt().invoke(2, -2));

  EXPECT_EQ(-2 <= 2, CmpOp::le().invoke(-2, 2));
  EXPECT_EQ(2 <= 2, CmpOp::le().invoke(2, 2));
  EXPECT_EQ(2 <= -2, CmpOp::le().invoke(2, -2));
}

TEST(conditions_test, cmpop_draw) {
  std::stringstream ss;
  ss << CmpOp::eq();
  EXPECT_EQ("==", ss.str());

  ss.str("");
  ss << CmpOp::ne();
  EXPECT_EQ("!=", ss.str());

  ss.str("");
  ss << CmpOp::gt();
  EXPECT_EQ(">", ss.str());

  ss.str("");
  ss << CmpOp::ge();
  EXPECT_EQ(">=", ss.str());

  ss.str("");
  ss << CmpOp::lt();
  EXPECT_EQ("<", ss.str());

  ss.str("");
  ss << CmpOp::le();
  EXPECT_EQ("<=", ss.str());
}
} // namespace
