namespace juice
{
  template <bool... B>
  struct conjunction;

  template <bool... B>
  struct conjunction <false, B...>
  {
    static constexpr bool value = false;
  };

  template <bool... B>
  struct conjunction<true, B...>
  {
    static constexpr bool value = conjunction<B...>::value;
  };

  template <>
  struct conjunction<>
  {
    static constexpr bool value = true;
  };
}
