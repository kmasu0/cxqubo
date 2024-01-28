#include "cxqubo/misc/shape.h"
#include "gtest/gtest.h"

using namespace cxqubo;

namespace {
class shape_test : public testing::Test {
  SpanOwner<unsigned> shape_owner;
  std::unique_ptr<unsigned> offset_check;

protected:
  void SetUp() override {
    shape_owner = {};
    offset_check.reset();
  }
  void TearDown() override {}

  ArrayShape make_shape(ConstSpan<unsigned> vals) {
    shape_owner = span_owner(vals);
    return shape_owner.as_const_span();
  }

  struct ShapedArray {
    using Item = unsigned;

    unsigned base = 0;
    ArrayShape shape_;

    void reset() { shape_ = ArrayShape(); }

    ArrayShape shape() const { return shape_; }
    size_t ndim() const { return shape_.size(); }
    ShapedArray remain(unsigned i) const {
      return ShapedArray{base + i * shape_.strides()[0], shape_.drop_front()};
    }
  };
  ShapedArray make_array(ConstSpan<unsigned> shape) {
    return {0, make_shape(shape)};
  }
};

TEST_F(shape_test, shape1d) {
  auto shape = make_shape(3);
  EXPECT_EQ(3, shape.nelements());

  EXPECT_TRUE(shape.inbounds(0));
  EXPECT_TRUE(shape.inbounds(2));
  EXPECT_FALSE(shape.inbounds(3));

  auto strides = shape.strides();
  EXPECT_EQ(1, strides[0]);

  EXPECT_EQ(1, shape.offset(1));
  EXPECT_EQ(2, shape.offset(2));
}

TEST_F(shape_test, shape2d) {
  auto shape = make_shape({3, 4});
  EXPECT_EQ(12, shape.nelements());

  auto strides = shape.strides();
  EXPECT_EQ(1, strides[1]);
  EXPECT_EQ(4, strides[0]);

  EXPECT_TRUE(shape.inbounds({0}));
  EXPECT_TRUE(shape.inbounds({2}));
  EXPECT_FALSE(shape.inbounds({3}));

  EXPECT_TRUE(shape.inbounds({2, 0}));
  EXPECT_TRUE(shape.inbounds({2, 3}));
  EXPECT_FALSE(shape.inbounds({3, 0}));
  EXPECT_FALSE(shape.inbounds({2, 4}));
}

TEST_F(shape_test, iter1d) {
  auto array = make_array({3});

  ShapedIter<ShapedArray> it(&array, 0);
  EXPECT_EQ(&array, it.array);
  EXPECT_EQ(0, it.index);

  auto partial = *it;
  auto parent = array.shape();
  auto child = partial.shape();
  EXPECT_FALSE(
      std::equal(parent.begin(), parent.end(), child.begin(), child.end()));
  EXPECT_EQ(0, partial.base);

  ++it;
  EXPECT_EQ(1, it.index);
  partial = *it;
  EXPECT_EQ(1, partial.base);

  ++it;
  EXPECT_EQ(2, it.index);
  partial = *it;
  EXPECT_EQ(2, partial.base);
}

TEST_F(shape_test, iter2d) {
  auto array = make_array({4, 5});

  ShapedIter<ShapedArray> it(&array, 0);
  EXPECT_EQ(&array, it.array);
  EXPECT_EQ(0, it.index);

  auto partial = *it;
  auto parent = array.shape();
  auto child = partial.shape();
  EXPECT_FALSE(
      std::equal(parent.begin(), parent.end(), child.begin(), child.end()));
  EXPECT_EQ(0, partial.base);

  ++it;
  EXPECT_EQ(1, it.index);
  partial = *it;
  EXPECT_EQ(5, partial.base);

  ShapedIter<ShapedArray> it2d(&partial, 0);
  EXPECT_NE(it2d.array, &array);
  EXPECT_EQ(0, it2d.index);

  auto partial2d = *it2d;
  EXPECT_EQ(5, partial2d.base);

  ++it2d;
  EXPECT_EQ(1, it2d.index);

  partial2d = *it2d;
  EXPECT_EQ(6, partial2d.base);

  ++it2d;
  EXPECT_EQ(2, it2d.index);

  partial2d = *it2d;
  EXPECT_EQ(7, partial2d.base);
}

TEST_F(shape_test, iter2d_for) {
  auto array = make_array({4, 5});

  unsigned cnt = 0;
  for (auto x : array_range(array)) {
    for (auto x2 : array_range(x)) {
      EXPECT_EQ(cnt, x2.base);
      ++cnt;
    }
  }
}
} // namespace
