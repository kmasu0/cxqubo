#include "cxqubo/misc/allocator.h"
#include "gtest/gtest.h"
#include <list>

using namespace cxqubo;

namespace {
class allocator_test : public testing::Test {
protected:
  static unsigned num_checked;
  static unsigned num_expects;

  void SetUp() override { num_checked = 0; }
  void TearDown() override { ASSERT_EQ(num_checked, num_expects); }

  static inline void check(int val, int expect) {
    ASSERT_EQ(expect, val);
    num_checked++;
  }

  struct V {
    int value = 0;
    int expect = 0;

    V(int v) : value(v) {}
    V(int v, int e) : value(v), expect(e) {}
    ~V() { check(); }

    void check() const {
      ASSERT_EQ(expect, value);
      num_checked++;
    }
  };

  struct Collection {
    std::list<V> simples;

    ~Collection() {}
  };
};

unsigned allocator_test::num_checked = 0;
unsigned allocator_test::num_expects = 0;

TEST_F(allocator_test, basics) {
  TypeBumpAllocator<int> alloc;
  int *a = (int *)alloc.allocate(1);
  int *b = (int *)alloc.allocate(10);
  int *c = (int *)alloc.allocate(1);
  *a = 1;
  b[0] = 2;
  b[9] = 2;
  *c = 3;
  EXPECT_EQ(1, *a);
  EXPECT_EQ(2, b[0]);
  EXPECT_EQ(2, b[9]);
  EXPECT_EQ(3, *c);
}

TEST_F(allocator_test, dtor) {
  TypeBumpAllocator<V> alloc;
  auto *v = new (alloc.allocate()) V(-10);
  v->expect = -10;
  num_expects = 1;
}
} // namespace
