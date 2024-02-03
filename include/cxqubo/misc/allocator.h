/// Allocators simplifying LLVM's allocators.
///
/// https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Support/Allocator.h
/// TODO:
/// https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Support/Recycler.h
/// https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Support/RecyclingAllocator.h

#ifndef CXQUBO_MISC_ALLOCATOR_H
#define CXQUBO_MISC_ALLOCATOR_H

#include "cxqubo/misc/type_traits.h"
#include <cstddef>
#include <new>
#include <vector>

namespace cxqubo {
namespace impl {
inline void *allocate_memory(size_t size, size_t alignment) {
  return ::operator new(size
#ifdef __cpp_aligned_new
                        ,
                        std::align_val_t(alignment)
#endif
  );
}

inline void deallocate_memory(void *ptr, size_t size, size_t alignment) {
  ::operator delete(ptr
#ifdef __cpp_sized_deallocation
                    ,
                    size
#endif
#ifdef __cpp_aligned_new
                    ,
                    std::align_val_t(alignment)
#endif
  );
}
} // namespace impl

template <class T, size_t NOBJECT = 4096 / sizeof(T), size_t NDELAY = 16>
class TypeBumpAllocator {
  /// Allocated blocks.
  std::vector<char *> blocks;
  /// Pointing beginning of current free space in the current block.
  char *cur = nullptr;
  /// Pointing end of the current block.
  char *end = nullptr;

  static inline constexpr size_t SIZE = sizeof(T);
  /// Size of a block.
  static inline constexpr size_t BLOCK_SIZE = sizeof(T[NOBJECT]);
  /// Limit of allocations.
  static inline constexpr size_t NLIMIT = (2 << 30) / BLOCK_SIZE;

  static inline size_t num_allocate_blocks(size_t n) {
    return std::min(NLIMIT, sizeof(1) << sizeof(n / NDELAY));
  }
  static inline size_t allocate_size(size_t n) {
    return BLOCK_SIZE * num_allocate_blocks(n);
  }

public:
  /// Ctor.
  TypeBumpAllocator() = default;
  /// Dtor calling dtors of each pointer and deallocating all blocks.
  ~TypeBumpAllocator() {
    delete_all();
    cur = nullptr;
    end = nullptr;
  }

  /// Allocate memory space for an instance without ctor.
  void *allocate(unsigned n = 1) {
    if (size_t(end - cur) < n * SIZE) {
      size_t bs = allocate_size(blocks.size());
      cur = reinterpret_cast<char *>(impl::allocate_memory(bs, alignof(T)));
      end = cur + bs;
      blocks.emplace_back(cur);
    }

    char *tmp = cur;
    cur = reinterpret_cast<char *>(reinterpret_cast<T *>(cur) + n);
    return tmp;
  }

  /// Deallocate memory space for an instance.
  void deallocate(void *p) {}

  /// Allocate and call ctor an object with the arguments.
  template <class... Args> T *create(Args &&...args) {
    static_assert(std::is_constructible_v<T, Args...>,
                  "create argument must be correct one.");
    return new (allocate()) T(args...);
  }

private:
  /// Call dtor for each pointer and delete all blocks.
  void delete_all() {
    auto destroy_elements = [](char *b, char *e) {
      for (char *p = b; p + sizeof(T) <= e; p += sizeof(T))
        reinterpret_cast<T *>(p)->~T();
    };
    for (unsigned i = 0, n = blocks.size(); i != n; ++i) {
      char *p = blocks[i];
      if (!p)
        continue;

      // Call dtors.
      char *b = p;
      char *e = p == blocks.back() ? cur : p + allocate_size(i);
      destroy_elements(b, e);

      // Remove the block.
      impl::deallocate_memory(p, allocate_size(i), alignof(T));
      blocks[i] = nullptr;
    }
  }
};
} // namespace cxqubo

#endif
