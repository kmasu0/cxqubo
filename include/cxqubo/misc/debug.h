#ifndef CXQUBO_MISC_DEBUG_H
#define CXQUBO_MISC_DEBUG_H

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <mutex>

namespace cxqubo {
inline uint32_t debug_level() {
  static uint32_t level = 0;
  static std::once_flag flag{};
  std::call_once(flag, []() {
    if (char *envstr = std::getenv("CXQUBO_DEBUG")) {
      level = std::stoi(envstr);
    }
  });

  return level;
}

inline bool debug_enabled() { return debug_level() != 0; }

inline std::ostream &odbg() { return std::cerr; }

inline std::ostream &odbg_indent() { return odbg() << "[CXQUBO] "; }

#if !defined(NDEBUG)
#define debug_code(X)                                                          \
  do {                                                                         \
    if (::cxqubo::debug_enabled()) {                                           \
      X;                                                                       \
    }                                                                          \
  } while (0)
#else
#define debug_code(X)
#endif
} // namespace cxqubo

#endif
