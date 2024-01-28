#include "cimod/vartypes.hpp"
#include "cxqubo/core/vartypes.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
TEST(vartypes_test, basics) {
  // Parse
  EXPECT_EQ(Vartype::SPIN, parse_vartype("SPIN"));
  EXPECT_EQ(Vartype::BINARY, parse_vartype("BINARY"));
  EXPECT_EQ(Vartype::NONE, parse_vartype("x"));
  EXPECT_EQ(Vartype::NONE, parse_vartype("binary"));

  // Correctness
  EXPECT_TRUE(is_correct_spin_value(-1, Vartype::SPIN));
  EXPECT_TRUE(is_correct_spin_value(1, Vartype::SPIN));
  EXPECT_FALSE(is_correct_spin_value(0, Vartype::SPIN));
  EXPECT_FALSE(is_correct_spin_value(2, Vartype::SPIN));
  EXPECT_FALSE(is_correct_spin_value(-2, Vartype::SPIN));

  EXPECT_TRUE(is_correct_spin_value(0, Vartype::BINARY));
  EXPECT_TRUE(is_correct_spin_value(1, Vartype::BINARY));
  EXPECT_FALSE(is_correct_spin_value(-1, Vartype::BINARY));
  EXPECT_FALSE(is_correct_spin_value(2, Vartype::BINARY));

  EXPECT_FALSE(is_correct_spin_value(-1, Vartype::NONE));
  EXPECT_FALSE(is_correct_spin_value(1, Vartype::NONE));
  EXPECT_FALSE(is_correct_spin_value(0, Vartype::NONE));

  // Convert
  EXPECT_EQ(0, convert_spin_value(-1, Vartype::SPIN, Vartype::BINARY));
  EXPECT_EQ(1, convert_spin_value(1, Vartype::SPIN, Vartype::BINARY));

  EXPECT_EQ(-1, convert_spin_value(0, Vartype::BINARY, Vartype::SPIN));
  EXPECT_EQ(1, convert_spin_value(1, Vartype::BINARY, Vartype::SPIN));

  EXPECT_EQ(-1, convert_spin_value(-1, Vartype::SPIN, Vartype::SPIN));
  EXPECT_EQ(1, convert_spin_value(1, Vartype::SPIN, Vartype::SPIN));

  EXPECT_EQ(0, convert_spin_value(0, Vartype::BINARY, Vartype::BINARY));
  EXPECT_EQ(1, convert_spin_value(1, Vartype::BINARY, Vartype::BINARY));
}

TEST(vartypes_test, draw) {
  std::stringstream ss;

  draw(ss, Vartype::NONE);
  EXPECT_EQ("None", ss.str());
  ss.str("");

  draw(ss, Vartype::SPIN);
  EXPECT_EQ("Spin", ss.str());
  ss.str("");

  draw(ss, Vartype::BINARY);
  EXPECT_EQ("Binary", ss.str());
  ss.str("");
}

TEST(vartypes_test, convertible_to_cimod) {
  EXPECT_EQ(unsigned(cimod::Vartype::NONE), unsigned(Vartype::NONE));
  EXPECT_EQ(unsigned(cimod::Vartype::SPIN), unsigned(Vartype::SPIN));
  EXPECT_EQ(unsigned(cimod::Vartype::BINARY), unsigned(Vartype::BINARY));
}
} // namespace
