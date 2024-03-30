#include "cxqubo/core/entity.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace cxqubo;

namespace {
struct V : public Entity<V, unsigned, 'v'> {};

TEST(entity_test, basics) {
  auto v = V::from(0);
  EXPECT_EQ(v.index(), 0);
  EXPECT_EQ(v.raw_id(), 1);
  EXPECT_FALSE(v.is_none());
  EXPECT_TRUE(v);

  auto v1 = V::from(~unsigned(0) - 1);
  EXPECT_EQ(v1.index(), ~unsigned(0) - 1);

  auto n = V::none();
  EXPECT_EQ(n.raw_id(), 0);
  EXPECT_EQ(n.index(), ~unsigned(0));
  EXPECT_FALSE(n);
}

TEST(entity_test, draw) {
  std::stringstream ss;
  ss << V::from(10);
  EXPECT_EQ(ss.str(), "v10");

  ss.str("");
  ss << V::none();
  EXPECT_EQ(ss.str(), "v(invalid)");
}
} // namespace
