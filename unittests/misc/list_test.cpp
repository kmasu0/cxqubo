#include "cxqubo/misc/list.h"
#include "gtest/gtest.h"

using namespace cxqubo;
namespace {
TEST(list_test, basics) {
  using Node = ForwardNode<int>;
  using Iter = ForwardNodeIter<int>;

  Node n(2);
  EXPECT_EQ(2, n.value);
  EXPECT_EQ(nullptr, n.next);

  Node n2(-2, &n);
  EXPECT_EQ(-2, n2.value);
  EXPECT_EQ(&n, n2.next);

  Iter it(&n2);
  EXPECT_EQ(&n2, it.node);
  ++it;
  EXPECT_EQ(&n, it.node);
  ++it;
  EXPECT_EQ(nullptr, it.node);
}
} // namespace
