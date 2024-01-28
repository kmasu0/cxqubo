#ifndef CXQUBO_MISC_ERROR_HANDLING_H
#define CXQUBO_MISC_ERROR_HANDLING_H

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace cxqubo {
inline std::ostream &oerr() noexcept { return std::cerr; }

inline std::string make_error_message(std::string_view msg,
                                      std::string_view file,
                                      unsigned line) noexcept {
  std::string result =
      std::string(file) + ":" + std::to_string(line) + ":" + std::string(msg);
  return result;
}

[[noreturn]] inline void report_unreachable_error(std::string_view msg,
                                                  std::string_view file,
                                                  unsigned line) noexcept {
  oerr().flush();
  oerr() << make_error_message(msg, file, line) << '\n';
  oerr().flush();
  abort();
}
} // namespace cxqubo

#define unreachable_code(MSG)                                                  \
  ::cxqubo::report_unreachable_error(MSG, __FILE__, __LINE__)

#endif
