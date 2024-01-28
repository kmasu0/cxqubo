#ifndef CXQUBO_MISC_VARIANT_H
#define CXQUBO_MISC_VARIANT_H

#include "cxqubo/misc/type_traits.h"
#include <cassert>
#include <variant>

namespace cxqubo {
/// Wrapper class to make it easy to use std::variant.
template <class... Ts> struct Variant : public std::variant<Ts...> {
  using Super = std::variant<Ts...>;
  using Super::Super;
  using Super::operator=;

  operator Super &() { return *this; }
  operator const Super &() const { return *this; }

  bool empty() const {
    if constexpr (contains_type<std::monostate, Ts...>)
      return is<std::monostate>();
    else
      return false;
  }

  template <class T> bool is() const {
    return std::holds_alternative<T>(*this);
  }
  template <class T> bool isnot() const {
    return !std::holds_alternative<T>(*this);
  }

  template <class T> bool is_any_of() const { return is<T>(); }
  template <class T, class T2, class... Tail> bool is_any_of() const {
    return is<T>() || is_any_of<T2, Tail...>();
  }

  template <class T> T &as() {
    assert(is<T>() && "the specified type is not included in variant");
    return std::get<T>(*this);
  }
  template <class T> const T &as() const {
    assert(is<T>() && "the specified type is not included in variant");
    return std::get<T>(*this);
  }

  template <class T> T *as_ptr_if() { return std::get_if<T>(this); }
  template <class T> const T *as_ptr_if() const { return std::get_if<T>(this); }
};
} // namespace cxqubo

#endif
