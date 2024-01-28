#ifndef CXQUBO_MISC_TYPE_TRAITS_H
#define CXQUBO_MISC_TYPE_TRAITS_H

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace cxqubo {
namespace impl {
template <int I, class T, class T0, class... Ts>
inline constexpr int type_index_of =
    std::is_same_v<T, T0> ? I : type_index_of<I + 1, T, Ts...>;

template <int I, class T, class T0>
inline constexpr int type_index_of<I, T, T0> = std::is_same_v<T, T0> ? I : -1;

template <size_t N, class T, class... Ts> struct indexed_type_of {
  using type = indexed_type_of<N - 1, Ts...>;
};
template <class T, class... Ts> struct indexed_type_of<0, T, Ts...> {
  using type = T;
};
template <size_t N, class T> struct indexed_type_of<N, T> {
  using type = void;
};
} // namespace impl

template <class T, class... Ts>
inline constexpr int type_index_of = impl::type_index_of<0, T, Ts...>;

template <class T, class... Ts>
inline constexpr bool contains_type = type_index_of<T, Ts...> >= 0;

template <int I, class... Ts>
requires(sizeof...(Ts) > 0) using indexed_type_of =
    typename impl::indexed_type_of<I, Ts...>::type;

template <class Fn> struct fn_type;
template <class R, class... Ts> struct fn_type<R (*)(Ts...)> {
  using ret_t = R;
  template <size_t I>
  using arg_t = typename std::tuple_element<I, std::tuple<Ts...>>::type;
  using fn_t = R (*)(Ts...);
  constexpr static const size_t num_args = sizeof...(Ts);
};

template <class T, size_t N> constexpr inline size_t array_length_of(T (&)[N]) {
  return N;
}
} // namespace cxqubo

#endif
