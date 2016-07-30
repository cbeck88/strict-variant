#include <tuple>

namespace juice
{
  template <typename... Types>
  class variant;

  template <typename T>
  class recursive_wrapper;

  static constexpr const size_t tuple_not_found = (size_t) -1;
  template <typename T, typename U> struct tuple_find;

  template <size_t N, typename T, typename... Types>
  struct tuple_find_helper;

  template <size_t N, typename T, typename First, typename... Types>
  struct tuple_find_helper<N, T, First, Types...> :
    public tuple_find_helper<N+1, T, Types...>
  {
  };

  template <size_t N, typename T, typename... Types>
  struct tuple_find_helper<N, T, recursive_wrapper<T>, Types...> :
    public std::integral_constant<std::size_t, N>
  {
  };

  template <size_t N, typename T, typename... Types>
  struct tuple_find_helper<N, T, T, Types...> :
    public std::integral_constant<std::size_t, N>
  {
  };

  template <size_t N, typename T>
  struct tuple_find_helper<N, T> :
    public std::integral_constant<decltype(tuple_not_found), tuple_not_found>
  {
  };

  template <typename T, typename... Types>
  struct tuple_find<T, std::tuple<Types...>> :
    public tuple_find_helper<0, T, Types...>
  {
  };

  template <typename T, typename T1, typename T2>
  struct tuple_find<T, std::pair<T1, T2>> :
    public tuple_find<T, std::tuple<T1, T2>>
  {
  };

  template <typename T, typename... Types>
  struct tuple_find<T, variant<Types...>> :
    public tuple_find<T, std::tuple<Types...>>
  {
  };

  template <typename T, typename U>
  constexpr size_t tuple_find_v = tuple_find<T, U>::value;
}

namespace std
{

  template <typename... Types>
  class tuple_size<juice::variant<Types...>> :
    public std::integral_constant<size_t, sizeof...(Types)>
  {
  };

  template <size_t I, typename... Types>
  class tuple_element<I, juice::variant<Types...>>
    : public tuple_element<I, tuple<Types...>> { };

}
