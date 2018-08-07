//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/**
 * A modern C++ tagged-union variant class.
 *
 * This effectively has similar functionality and mostly the same interface as
 * boost::variant, but replaces the implementation with C++11 features.
 *
 * Fully supports move semantics, and arbitrary numbers of types.
 * A major advantage is the use of variadic templates instead of the
 * boost mpl / boost macro library, which make compilation take a long
 * time and can be hard to follow, at least for the uninitiated.
 *
 * This code is derived from (an early version of) Jarryd Beck's variant:
 *   https://github.com/jarro2783/thenewcpp
 */

#include <strict_variant/filter_overloads.hpp>
#include <strict_variant/mpl/find_with.hpp>
#include <strict_variant/mpl/nonstd_traits.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <strict_variant/mpl/typelist.hpp>
#include <strict_variant/mpl/ulist.hpp>
#include <strict_variant/recursive_wrapper.hpp>
#include <strict_variant/safely_constructible.hpp>
#include <strict_variant/variant_detail.hpp>
#include <strict_variant/variant_dispatch.hpp>
#include <strict_variant/variant_fwd.hpp>
#include <strict_variant/variant_storage.hpp>

#include <type_traits>
#include <utility>

// #define STRICT_VARIANT_ASSUME_MOVE_NOTHROW
// #define STRICT_VARIANT_ASSUME_COPY_NOTHROW
// #define STRICT_VARIANT_DEBUG

#ifdef STRICT_VARIANT_DEBUG
#include <cassert>

#define STRICT_VARIANT_ASSERT(X, C)                                                                \
  do {                                                                                             \
    assert((X) && C);                                                                              \
  } while (0)

#else // STRICT_VARIANT_DEBUG

#define STRICT_VARIANT_ASSERT(X, C)                                                                \
  do {                                                                                             \
  } while (0)

#endif // STRICT_VARIANT_DEBUG

namespace strict_variant {

/***
 * Trait to detect specializations of variant
 */
template <typename T>
struct is_variant : std::false_type {};

template <typename First, typename... Types>
struct is_variant<variant<First, Types...>> : std::true_type {};

/***
 * Tag used in tag-dispatch with emplace-ctor
 */
template <typename T>
struct emplace_tag {};

/***
 * Class variant
 */
template <typename First, typename... Types>
class variant {

private:
  /***
   * Check noexcept status of special member functions of our types
   */
  static_assert(mpl::All_Have<std::is_nothrow_destructible, First>::value,
                "All types in this variant type must be nothrow destructible");
  static_assert(mpl::All_Have<std::is_nothrow_destructible, Types...>::value,
                "All types in this variant type must be nothrow destructible");

  /***
   * Prohibit references
   */
  static_assert(mpl::None_Have<std::is_reference, First>::value,
                "Cannot store references in this variant, use `std::reference_wrapper`");
  static_assert(mpl::None_Have<std::is_reference, Types...>::value,
                "Cannot store references in this variant, use `std::reference_wrapper`");

  /***
   * Data members
   */

  using storage_t = detail::storage<First, Types...>;
  storage_t m_storage;

  int m_which;

  /***
   * Initialize and destroy
   */
  void destroy() noexcept {
    destroyer d;
    this->apply_visitor_internal(d);
  }

  template <std::size_t index, typename... Args>
  void initialize(Args &&... args) noexcept(
    noexcept(static_cast<storage_t *>(nullptr)->template initialize<index>(
      std::forward<Args>(std::declval<Args>())...))) {
    m_storage.template initialize<index>(std::forward<Args>(args)...);
    this->m_which = static_cast<int>(index);
  }

  /***
   * (Type-changing) Assignment
   */
  template <std::size_t index, typename Rhs>
  void assign(Rhs && rhs) {
    constexpr bool assume_nothrow_init =
      std::is_lvalue_reference<Rhs>::value
        ? detail::variant_noexcept_helper<First, Types...>::assume_copy_nothrow
        : detail::variant_noexcept_helper<First, Types...>::assume_move_nothrow;

    // This is a recursive_wrapper if that is what storage is using internally
    using temp_t = typename storage_t::template value_t<index>;

    // Three cases:
    // 1) Already had an RHS type in the variant. Use assignment directly. Must pierce
    // recursive_wrapper.
    // 2) Must change type, but initializing the new value is noexcept. Can destroy and do it
    // directly.
    // 3) Must change type, and initializing the new value may throw. Do it on the stack, and then
    // move into storage.

    static_assert(noexcept(this->destroy()), "Noexcept assumption failed!");

    if (this->which() == index) {
      m_storage.template get_value<index>(detail::false_{}) = std::forward<Rhs>(rhs);
    } else if (assume_nothrow_init || noexcept(this->initialize<index>(std::forward<Rhs>(rhs)))) {
      this->destroy();
      this->initialize<index>(std::forward<Rhs>(rhs));
    } else {
      static_assert(detail::variant_noexcept_helper<First, Types...>::assume_move_nothrow
                      || noexcept(this->initialize<index>(std::declval<temp_t>())),
                    "Noexcept assumption failed!");

      temp_t tmp(std::forward<Rhs>(rhs));      // may throw
      this->destroy();                         // nothrow
      this->initialize<index>(std::move(tmp)); // nothrow
    }
  }

  /***
   * Used for internal visitors
   */
  template <typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) -> typename Visitor::result_type {
    // Implementation note:
    // `detail::true_` here indicates that the visit is internal and we should
    // NOT pierce `recursive_wrapper`.
    return detail::visitor_dispatch<detail::true_, 1 + sizeof...(Types)>{}(static_cast<unsigned>(m_which), m_storage,
                                                                           visitor);
  }

  /***
   * find_which is used with non-T&& ctors to figure out what "which" should be
   * used for a given type
   */
  template <typename Rhs>
  struct find_which {
    static constexpr std::size_t value =
      mpl::Find_With<detail::same_modulo_const_ref_wrapper<Rhs>::template prop, First,
                     Types...>::value;
    static_assert(value < (sizeof...(Types) + 1), "No match for value");
  };

  /***
   * Visitors used to implement special member functions and such
   */
  struct constructor;
  struct assigner;
  struct destroyer;
  struct swapper;

  /***
   * initializer_slot is a template function which figures out which slot
   * should be used when initializing or assigning from an object of type T &&.
   *
   * This is used in T && ctor and T && operator =.
   * It works by creating an appropriate overloaded function object and
   * applying it to an argument of type T &&.
   */

  // init_helper trait maps an index to the corresponding value type, by
  // interrogating storage_t to ensure consistency.
  template <unsigned idx>
  struct init_helper {
    using type = unwrap_type_t<typename storage_t::template value_t<idx>>;
  };

  // Initializer leaf is a function object
  // Note: Actually the "forbidding" is accomplished using filter_overloads
  // below, we don't need to use SFINAE on operator() here.
  template <typename T, unsigned idx>
  struct initializer_leaf {
    using target_type = typename init_helper<idx>::type;

    constexpr std::integral_constant<unsigned, idx> operator()(target_type) const { return {}; }
  };

  // Main object, created using inheritance
  // T should be a forwarding reference
  template <typename T, typename UL>
  struct initializer_base;

  template <typename T, unsigned u>
  struct initializer_base<T, mpl::ulist<u>> : initializer_leaf<T, u> {
    using initializer_leaf<T, u>::operator();
  };

  template <typename T, unsigned u, unsigned... us>
  struct initializer_base<T, mpl::ulist<u, us...>> : initializer_leaf<T, u>,
                                                     initializer_base<T, mpl::ulist<us...>> {
    using initializer_leaf<T, u>::operator();
    using initializer_base<T, mpl::ulist<us...>>::operator();
  };

  template <typename T>
  struct initializer
    : initializer_base<T,
                       typename filter_overloads<T, mpl::ulist_map_t<init_helper,
                                                                     mpl::count_t<sizeof...(Types)
                                                                                  + 1>>>::type> {};

  // Interface, this is what is actually used in T && ctor and assignment op
  template <typename T>
  static constexpr unsigned initializer_slot() {
    return decltype(initializer<T>{}(std::declval<T>()))::value;
  }

public:
  ~variant() noexcept { this->destroy(); }

  // Constructors
  // Note: We use detail:: instead of std:: for trait because we handle
  // recursive_wrapper<T> specially
  // TODO: It would be nice if we actually SFINAED the definitions of these
  // special member functions, following akrzemi1's technical description:
  // https://akrzemi1.wordpress.com/2015/03/02/a-conditional-copy-constructor/
  variant() noexcept(detail::is_nothrow_default_constructible<First>::value);

  variant(const variant & rhs) noexcept(
    detail::variant_noexcept_helper<First, Types...>::nothrow_copy_ctors);

  variant(variant && rhs) noexcept(
    detail::variant_noexcept_helper<First, Types...>::nothrow_move_ctors);

  /// Forwarding-reference ctor, construct a variant from one of its value
  /// types, using overload resolution. See documentation.
  template <typename T,
            typename =
              mpl::enable_if_t<!is_variant<mpl::remove_const_t<mpl::remove_reference_t<T>>>::value>>
  variant(T && t);

  /// "Generalizing Ctor"
  /// Allow constructing from a variant over a subset of our types
  /// (Boost variant does this, and we need it to comfortably interact with
  /// spirit)
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant(const variant<OFirst, OTypes...> & other) noexcept(
    detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_copy_ctors);

  /// "Generalizing" move ctor, similar as above
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant(variant<OFirst, OTypes...> && other) noexcept(
    detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_move_ctors);

  // Emplace ctor. Used to explicitly specify the type of the variant, and
  // invoke an arbitrary ctor of that type.
  template <typename T, typename... Args>
  explicit variant(emplace_tag<T>,
                   Args &&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value);

  /***
   * Modifiers
   */

  // Assignment
  variant & operator=(const variant & rhs) noexcept(
    detail::variant_noexcept_helper<First, Types...>::nothrow_copy_assign);

  variant & operator=(variant && rhs) noexcept(
    detail::variant_noexcept_helper<First, Types...>::nothrow_move_assign);

  // Forwarding reference assignment
  template <typename T,
            typename =
              mpl::enable_if_t<!is_variant<mpl::remove_const_t<mpl::remove_reference_t<T>>>::value>>
  variant & operator=(T && t);

  // Generalizing assignment
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant & operator=(const variant<OFirst, OTypes...> & other) noexcept(
    detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_copy_assign);

  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant & operator=(variant<OFirst, OTypes...> && other) noexcept(
    detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_move_assign);

  // Emplace operation
  template <std::size_t index, typename... Args>
  mpl::enable_if_t<!std::is_nothrow_constructible<typename storage_t::template value_t<index>,
                                                  Args...>::value>
  emplace(Args &&... args) noexcept(false);

  template <std::size_t index, typename... Args>
  mpl::enable_if_t<std::is_nothrow_constructible<typename storage_t::template value_t<index>,
                                                 Args...>::value>
  emplace(Args &&... args) noexcept;

  // Emplace with explicitly specified type -- makes a call to index version
  template <typename T, typename... Args>
  void emplace(Args &&... args) noexcept(noexcept(
    static_cast<variant *>(nullptr)->emplace<find_which<T>::value>(std::forward<Args>(args)...))) {
    constexpr std::size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    this->emplace<idx>(std::forward<Args>(args)...);
  }

  // Swap operation
  // Optimized in case of `recursive_wrapper` to use a pointer move.
  void swap(variant & other) noexcept;

  /***
   * Accessors
   */

  int which() const noexcept { return m_which; }

  // get
  template <typename T>
  T * get() noexcept {
    constexpr std::size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    return this->get<idx>();
  }

  template <typename T>
  const T * get() const noexcept {
    constexpr std::size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    return this->get<idx>();
  }

  // get with integer index
  template <std::size_t idx>
  auto get() noexcept
    -> decltype(&static_cast<storage_t *>(nullptr)->template get_value<idx>(detail::false_{})) {
    if (idx == m_which) {
      return &m_storage.template get_value<idx>(detail::false_{});
    } else {
      return nullptr;
    }
  }

  template <std::size_t idx>
  auto get() const noexcept -> decltype(
    &static_cast<const storage_t *>(nullptr)->template get_value<idx>(detail::false_{})) {
    if (idx == m_which) {
      return &m_storage.template get_value<idx>(detail::false_{});
    } else {
      return nullptr;
    }
  }

  /***
   * Visitation
   */

  // Implementation details for apply_visitor
  // private:
  using dispatcher_t = detail::visitor_dispatch<detail::false_, 1 + sizeof...(Types)>;

#define APPLY_VISITOR_IMPL_BODY                                                                    \
  dispatcher_t{}(static_cast<unsigned>(visitable.which()), std::forward<Visitable>(visitable).m_storage,                  \
                 std::forward<Visitor>(visitor))

  // Visitable is assumed to be, forwarding reference to this type.
  template <typename Visitor, typename Visitable>
  static auto apply_visitor_impl(Visitor && visitor,
                                 Visitable && visitable) noexcept(noexcept(APPLY_VISITOR_IMPL_BODY))
    -> decltype(APPLY_VISITOR_IMPL_BODY) {
    static_assert(std::is_same<const variant, const mpl::remove_reference_t<Visitable>>::value,
                  "Misuse of apply_visitor_impl!");
    return APPLY_VISITOR_IMPL_BODY;
  }

#undef APPLY_VISITOR_IMPL_BODY

  // public:
  // C++17 visit syntax
  template <typename V>
    auto visit(V && v)
    & noexcept(noexcept(apply_visitor_impl(std::forward<V>(v), *static_cast<variant *>(nullptr))))
        -> decltype(apply_visitor_impl(std::forward<V>(v), *this)) {
    return apply_visitor_impl(std::forward<V>(v), *this);
  }

  template <typename V>
  auto visit(V && v) const & noexcept(
    noexcept(apply_visitor_impl(std::forward<V>(v), *static_cast<const variant *>(nullptr))))
    -> decltype(apply_visitor_impl(std::forward<V>(v), *this)) {
    return apply_visitor_impl(std::forward<V>(v), *this);
  }

  template <typename V>
    auto visit(V && v)
    && noexcept(noexcept(apply_visitor_impl(std::forward<V>(v),
                                            std::move(*static_cast<variant *>(nullptr)))))
         -> decltype(apply_visitor_impl(std::forward<V>(v), std::move(*this))) {
    return apply_visitor_impl(std::forward<V>(v), std::move(*this));
  }
};

/***
 * apply one visitor function. `boost::variant` syntax.
 * This is the basic version, used in implementation of multivisitation.
 */
#define APPLY_VISITOR_BODY                                                                         \
  mpl::remove_reference_t<Visitable>::apply_visitor_impl(std::forward<Visitor>(visitor),           \
                                                         std::forward<Visitable>(visitable))
template <typename Visitor, typename Visitable>
auto
apply_visitor(Visitor && visitor, Visitable && visitable) noexcept(noexcept(APPLY_VISITOR_BODY))
  -> decltype(APPLY_VISITOR_BODY) {
  return APPLY_VISITOR_BODY;
}

#undef APPLY_VISITOR_BODY

/***
 * strict_variant::get function (same semantics as boost::get with pointer type)
 */
template <typename T, typename... Types>
T *
get(variant<Types...> * var) noexcept {
  return var->template get<T>();
}

template <typename T, typename... Types>
const T *
get(const variant<Types...> * var) noexcept {
  return var->template get<T>();
}

// Using integer index
template <std::size_t idx, typename... Types>
auto
get(variant<Types...> * var) noexcept
  -> decltype(static_cast<variant<Types...> *>(nullptr)->template get<idx>()) {
  return var->template get<idx>();
}

template <std::size_t idx, typename... Types>
auto
get(const variant<Types...> * var) noexcept
  -> decltype(static_cast<const variant<Types...> *>(nullptr)->template get<idx>()) {
  return var->template get<idx>();
}

/// If a variant has type T, then get a reference to it,
/// otherwise, create a new T default value in the variant
/// and return a reference to the new value.
template <typename T, typename... Types>
T &
get_or_default(variant<Types...> & v,
               T def = {}) noexcept(std::is_nothrow_move_constructible<T>::value) {
  T * t = strict_variant::get<T>(&v);
  if (!t) {
    v.template emplace<T>(std::move(def));
    t = strict_variant::get<T>(&v);
    STRICT_VARIANT_ASSERT(t, "Move assignment to a variant failed to change its type!");
  }
  return *t;
}

/***
 * Trait to add the wrapper if a type is not no-throw move constructible
 */

//[ strict_variant_wrap_if_throwing_move
template <typename T, typename = mpl::enable_if_t<std::is_nothrow_destructible<T>::value
                                                  && !std::is_reference<T>::value>>
struct wrap_if_throwing_move {
  using type = typename std::conditional<std::is_nothrow_move_constructible<T>::value, T,
                                         recursive_wrapper<T>>::type;
};

template <typename T>
using wrap_if_throwing_move_t = typename wrap_if_throwing_move<T>::type;
//]

//[ strict_variant_easy_variant
template <typename... Ts>
using easy_variant = variant<wrap_if_throwing_move_t<Ts>...>;
//]

/***
 * Implementation details of private visitors
 */
template <typename First, typename... Types>
struct variant<First, Types...>::constructor {
  typedef void result_type;

  explicit constructor(variant & self)
    : m_self(self) {}

  template <typename T>
  void operator()(T && rhs) const {
    constexpr std::size_t index = find_which<mpl::remove_reference_t<T>>::value;
    m_self.template initialize<index>(std::forward<T>(rhs));
  }

private:
  variant & m_self;
};

// assigner
template <typename First, typename... Types>
struct variant<First, Types...>::assigner {

  static_assert(detail::variant_noexcept_helper<First, Types...>::assignable,
                "All types in this variant must be nothrow move constructible or placed in a "
                "recursive_wrapper, or the variant cannot be assigned!");

  explicit assigner(variant & self)
    : m_self(self) {}

  template <typename Rhs>
  void operator()(Rhs && rhs) const {
    constexpr std::size_t index = find_which<mpl::remove_reference_t<Rhs>>::value;
    m_self.template assign<index>(std::forward<Rhs>(rhs));
  }

private:
  variant & m_self;
};

// destroyer
template <typename First, typename... Types>
struct variant<First, Types...>::destroyer {
  typedef void result_type;

  template <typename T>
  void operator()(T & t) const noexcept {
    t.~T();
  }
};

/***
 * Implementation details of ctors
 */

#define STRICT_VARIANT_ASSERT_WHICH_INVARIANT                                                      \
  STRICT_VARIANT_ASSERT(static_cast<unsigned>(this->which()) < sizeof...(Types) + 1,               \
                        "Postcondition failed!")

template <typename First, typename... Types>
variant<First, Types...>::variant() noexcept(
  detail::is_nothrow_default_constructible<First>::value) {
  static_assert(std::is_default_constructible<First>::value,
                "First type must be default constructible or variant is not!");
  this->initialize<0>();
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
}

// Special member functions
template <typename First, typename... Types>
variant<First, Types...>::variant(const variant & rhs) noexcept(
  detail::variant_noexcept_helper<First, Types...>::nothrow_copy_ctors) {
  constructor c(*this);
  apply_visitor(c, rhs);
  STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
}

// Note: noexcept is enforced by static_assert in move_constructor visitor
template <typename First, typename... Types>
variant<First, Types...>::variant(variant && rhs) noexcept(
  detail::variant_noexcept_helper<First, Types...>::nothrow_move_ctors) {
  constructor mc(*this);
  apply_visitor(mc, std::move(rhs));
  STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
}

template <typename First, typename... Types>
variant<First, Types...> &
variant<First, Types...>::operator=(const variant & rhs) noexcept(
  detail::variant_noexcept_helper<First, Types...>::nothrow_copy_assign) {
  assigner a(*this);
  apply_visitor(a, rhs);
  STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  return *this;
}

// Note: We want to pierce the recursive_wrapper here, if we move it then
template <typename First, typename... Types>
variant<First, Types...> &
variant<First, Types...>::operator=(variant && rhs) noexcept(
  detail::variant_noexcept_helper<First, Types...>::nothrow_move_assign) {
  assigner ma(*this);
  apply_visitor(ma, std::move(rhs));
  STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  return *this;
}

/// Forwarding-reference ctor
template <typename First, typename... Types>
template <typename T, typename>
variant<First, Types...>::variant(T && t) {
  static_assert(!std::is_same<variant &, mpl::remove_const_t<T>>::value,
                "why is variant(T&&) instantiated with a variant? why was a special "
                "member function not selected?");
  constexpr unsigned idx = initializer_slot<T>();
  this->initialize<idx>(std::forward<T>(t));
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
}

/// Forwarding-reference assignment

template <typename First, typename... Types>
template <typename T, typename>
variant<First, Types...> &
variant<First, Types...>::operator=(T && t) {
  constexpr unsigned idx = initializer_slot<T>();
  this->assign<idx>(std::forward<T>(t));
  return *this;
}

/// "Generalizing Ctor"
/// Allow constructing from a variant over a subset of our types
/// (Boost variant does this, and we need it to comfortably interact with
/// spirit)
template <typename First, typename... Types>
template <typename OFirst, typename... OTypes, typename Enable>
variant<First, Types...>::variant(const variant<OFirst, OTypes...> & other) noexcept(
  detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_copy_ctors) {
  constructor c(*this);
  apply_visitor(c, other);
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
}

/// "Generalizing" move ctor, similar as above
template <typename First, typename... Types>
template <typename OFirst, typename... OTypes, typename Enable>
variant<First, Types...>::variant(variant<OFirst, OTypes...> && other) noexcept(
  detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_move_ctors) {
  constructor c(*this);
  apply_visitor(c, std::move(other));
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
}

// Generalizing assignments
template <typename First, typename... Types>
template <typename OFirst, typename... OTypes, typename Enable>
variant<First, Types...> &
variant<First, Types...>::operator=(const variant<OFirst, OTypes...> & other) noexcept(
  detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_copy_assign) {
  assigner a(*this);
  apply_visitor(a, other);
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  return *this;
}

template <typename First, typename... Types>
template <typename OFirst, typename... OTypes, typename Enable>
variant<First, Types...> &
variant<First, Types...>::operator=(variant<OFirst, OTypes...> && other) noexcept(
  detail::variant_noexcept_helper<OFirst, OTypes...>::nothrow_move_assign) {
  assigner a(*this);
  apply_visitor(a, std::move(other));
  STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  return *this;
}

// Emplace ctor. Used to explicitly specify the type of the variant, and
// invoke an arbitrary ctor of that type.
template <typename First, typename... Types>
template <typename T, typename... Args>
variant<First, Types...>::variant(emplace_tag<T>, Args &&... args) noexcept(
  std::is_nothrow_constructible<T, Args...>::value) {
  constexpr std::size_t idx = find_which<T>::value;
  static_assert(idx < sizeof...(Types) + 1, "Requested type is not a member of this variant type");

  this->initialize<idx>(std::forward<Args>(args)...);
}

// Emplace operation
// In this operation the user explicitly specifies the desired type as
// template parameter, which must be one of the variant types, modulo const
// and recursive wrapper.
// There are two overloads:
//   when the invoked constructor is noexcept, we destroy the current value
//     and reinitialize in-place.
//   when the invoked constructor is not noexcept, we use a move for safety.
template <typename First, typename... Types>
template <std::size_t idx, typename... Args>
auto
variant<First, Types...>::emplace(Args &&... args) noexcept(false)
  -> mpl::enable_if_t<!std::is_nothrow_constructible<typename storage_t::template value_t<idx>,
                                                     Args...>::value> {
  using temp_t = typename storage_t::template value_t<idx>;
  static_assert(
    std::is_nothrow_move_constructible<temp_t>::value,
    "To use emplace, either the invoked ctor or the move ctor of value type must be noexcept.");
  temp_t temp(std::forward<Args>(args)...);
  this->emplace<idx>(std::move(temp));
}

template <typename First, typename... Types>
template <std::size_t idx, typename... Args>
auto
variant<First, Types...>::emplace(Args &&... args) noexcept
  -> mpl::enable_if_t<std::is_nothrow_constructible<typename storage_t::template value_t<idx>,
                                                    Args...>::value> {
  static_assert(idx < sizeof...(Types) + 1, "Requested type is not a member of this variant type");
  static_assert(noexcept(this->initialize<idx>(std::forward<Args>(args)...)),
                "Noexcept assumption failed!");

  this->destroy();
  this->initialize<idx>(std::forward<Args>(args)...);
}

// Swap

template <typename First, typename... Types>
struct variant<First, Types...>::swapper {
  using var_t = variant<First, Types...>;

  swapper(var_t & lhs_var, var_t & rhs_var)
    : lhs_(lhs_var)
    , rhs_(rhs_var) {}

  void do_swap() const noexcept { lhs_.apply_visitor_internal(*this); }

  // First visit is *this, to lhs_var
  // Second visit is to rhs_var
  template <typename T>
  void operator()(T & first_visit) const noexcept {
    second_visitor<T> v{lhs_, rhs_, first_visit};
    rhs_.apply_visitor_internal(v);
  }

  template <typename T>
  struct second_visitor {
    var_t & first_var_;
    var_t & second_var_;
    T & first_visit_;

    explicit second_visitor(var_t & first_var, var_t & second_var, T & first_visit)
      : first_var_(first_var)
      , second_var_(second_var)
      , first_visit_(first_visit) {}

    // If both give us a T, and T is noexcept swappable, then do that
    mpl::enable_if_t<mpl::is_nothrow_swappable<T>::value> operator()(T & second_visit) const
      noexcept {
      using std::swap;
      swap(first_visit_, second_visit);
    }

    // swap using a move
    template <typename U>
    void operator()(U & second_visit) const noexcept {
      constexpr std::size_t t_idx = var_t::find_which<T>::value;
      constexpr std::size_t u_idx = var_t::find_which<U>::value;

      STRICT_VARIANT_ASSERT(t_idx == first_var_.which(), "Bad access during swap!");
      STRICT_VARIANT_ASSERT(u_idx == second_var_.which(), "Bad access during swap!");

      T temp{std::move(first_visit_)};
      first_var_.destroy();
      first_var_.initialize<u_idx>(std::move(second_visit));
      second_var_.destroy();
      second_var_.initialize<t_idx>(std::move(temp));
    }
  };

private:
  var_t & lhs_;
  var_t & rhs_;
};

template <typename First, typename... Types>
void
variant<First, Types...>::swap(variant & other) noexcept {
  swapper s{*this, other};
  s.do_swap();
}

// Operator ==, !=

// equality check
// This is essentially a multivisitor, but we do the boiler-plate manually to
// avoid including extra stuff.
template <typename First, typename... Types>
struct eq_checker {
  typedef bool result_type;

  using var_t = variant<First, Types...>;

  eq_checker(const var_t & lhs_variant)
    : lhs_v(lhs_variant) {}

  // After we've visited the first value, store it in a function object.
  // second_visitor<T> is applied to the variant we are passed in ctor.
  template <typename T>
  struct second_visitor {
    const T & r;

    bool operator()(const T & l) const { return l == r; }
    template <typename U>
    bool operator()(const U &) const {
      STRICT_VARIANT_ASSERT(false, "Should be unreachable!");
      return false;
    }
  };

  template <typename Rhs>
  bool operator()(const Rhs & rhs) const {
    return apply_visitor(second_visitor<Rhs>{rhs}, lhs_v);
  }

private:
  const var_t & lhs_v;
};

template <typename First, typename... Types>
inline bool
operator==(const variant<First, Types...> & lhs, const variant<First, Types...> & rhs) {
  if (lhs.which() != rhs.which()) { return false; }
  eq_checker<First, Types...> eq{lhs};
  return apply_visitor(eq, rhs);
}

template <typename First, typename... Types>
inline bool
operator!=(const variant<First, Types...> & lhs, const variant<First, Types...> & rhs) {
  return !(lhs == rhs);
}

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
#undef STRICT_VARIANT_ASSERT_WHICH_INVARIANT
