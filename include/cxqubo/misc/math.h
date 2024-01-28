#ifndef CXQUBO_MISC_MATH_H
#define CXQUBO_MISC_MATH_H

#include "cxqubo/misc/error_handling.h"
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace cxqubo {
template <std::integral T0, std::integral T1>
inline constexpr T0 divide_ceil(T0 n, T1 divisor) {
  return (n + (T0(divisor) - 1)) / T0(divisor);
}
inline bool is_pow2(uint64_t n) { return n == 0 ? false : (n & (n - 1)) == 0; }
template <size_t N> inline bool is_pow2_const() {
  return N == 0 ? false : (N & (N - 1)) == 0;
}
inline bool is_aligned(uintptr_t p, size_t n) { return (p & (n - 1)) == 0; }
template <size_t N> inline bool is_aligned_const(uintptr_t p) {
  static_assert(is_pow2_const<N>(), "N must be power of 2!");
  return (p & (N - 1)) == 0;
}
inline size_t align_to(size_t n, unsigned align) {
  return divide_ceil<size_t>(n, align) * align;
}
inline size_t offset_to_align(size_t n, unsigned align) {
  return align_to(n, align) - n;
}
inline size_t offset_to_alignaddr(const void *addr, unsigned align) {
  return offset_to_align(uintptr_t(addr), align);
}
inline uintptr_t align_address(const void *addr, unsigned align) {
  uintptr_t v = uintptr_t(addr);
  assert(uintptr_t(v + align - 1) >= v && "align calculation overflow");
  return align_to(v, align);
}
inline unsigned log2_32(uint32_t n) {
  assert(n != 0 && "argument must not be zero!");
  return 31 - std::countl_zero(n);
}
inline unsigned log2_64(uint64_t n) {
  assert(n != 0 && "argument must not be zero!");
  return 63 - std::countl_zero(n);
}
inline unsigned log2_32_ceil(uint32_t n) { return 32 - std::countl_zero(n); }
inline unsigned log2_64_ceil(size_t n) { return 64 - std::countl_zero(n); }

inline constexpr size_t aligned_sizeof(size_t n) {
  if (n == 1)
    return 1;
  if (n < 2)
    return 2;
  if (n < 4)
    return 4;
  if (n < 8)
    return 8;
  return 8 * divide_ceil(n, 8);
}
} // namespace cxqubo

#endif
