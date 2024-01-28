#include "cxqubo/core/conditions.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
TEST(conditions_test, cmpop_basics) {
  EXPECT_EQ(0, CmpOp::INVALID);
  EXPECT_EQ(CmpOp::EQ, CmpOp::eq());
  EXPECT_EQ(CmpOp::NE, CmpOp::ne());
  EXPECT_EQ(CmpOp::GT, CmpOp::gt());
  EXPECT_EQ(CmpOp::GE, CmpOp::ge());
  EXPECT_EQ(CmpOp::LT, CmpOp::lt());
  EXPECT_EQ(CmpOp::LE, CmpOp::le());

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
  CmpOp::eq().draw(ss);
  EXPECT_EQ("==", ss.str());

  ss.str("");
  CmpOp::ne().draw(ss);
  EXPECT_EQ("!=", ss.str());

  ss.str("");
  CmpOp::gt().draw(ss);
  EXPECT_EQ(">", ss.str());

  ss.str("");
  CmpOp::ge().draw(ss);
  EXPECT_EQ(">=", ss.str());

  ss.str("");
  CmpOp::lt().draw(ss);
  EXPECT_EQ("<", ss.str());

  ss.str("");
  CmpOp::le().draw(ss);
  EXPECT_EQ("<=", ss.str());
}
} // namespace
