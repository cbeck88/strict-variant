//  (C) Copyright 2016 Christopher Beck

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

#include <strict_variant/mpl/find_with.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <strict_variant/mpl/typelist.hpp>
#include <strict_variant/mpl/ulist.hpp>
#include <strict_variant/recursive_wrapper.hpp>
#include <strict_variant/safely_constructible.hpp>
#include <strict_variant/variant_detail.hpp>
#include <strict_variant/variant_dispatch.hpp>
#include <strict_variant/variant_fwd.hpp>
#include <strict_variant/variant_storage.hpp>

#include <new>
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
 * Class variant
 */
template <typename First, typename... Types>
class variant {

private:
  /***
   * Configuration
   */

  // Treat all input types as if they were nothrow moveable,
  // regardless of their noexcept declarations.
  // (Note: It is a core assumption of the variant that these operations don't
  //  throw, you will get UB and likely an immediate crash if they do throw.
  //  Only use this as a workaround for e.g. old code which is not
  //  noexcept-correct. For instance if you require compatibility with a
  //  gcc-4-series version of libstdc++ and std::string isn't noexcept.)
  static constexpr bool assume_move_nothrow =
#ifdef STRICT_VARIANT_ASSUME_MOVE_NOTHROW
    true;
#else
    false;
#endif

  // Treat all input types as if they were nothrow copyable,
  // regardless of thier noexcept declarations.
  // (Note: This is usually quite risky, only appropriate in projects which
  //  assume already that dynamic memory allocation will never fail, and want to
  //  go as fast as possible given that assumption.)
  static constexpr bool assume_copy_nothrow =
#ifdef STRICT_VARIANT_ASSUME_COPY_NOTHROW
    true;
#else
    false;
#endif

  /***
   * Check noexcept status of special member functions of our types
   */

  static_assert(mpl::All_Have<std::is_nothrow_destructible, First>::value,
                "All types in this variant type must be nothrow destructible");
  static_assert(mpl::All_Have<std::is_nothrow_destructible, Types...>::value,
                "All types in this variant type must be nothrow destructible");

  static constexpr bool nothrow_move_ctors =
    assume_move_nothrow
    || mpl::All_Have<detail::is_nothrow_moveable, First, Types...>::value;

  static constexpr bool nothrow_copy_ctors =
    assume_copy_nothrow
    || mpl::All_Have<detail::is_nothrow_copyable, First, Types...>::value;

  static constexpr bool nothrow_move_assign =
    nothrow_move_ctors && mpl::All_Have<detail::is_nothrow_move_assignable, First, Types...>::value;

  static constexpr bool nothrow_copy_assign =
    nothrow_copy_ctors && mpl::All_Have<detail::is_nothrow_copy_assignable, First, Types...>::value;

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
  void destroy() {
    destroyer d;
    this->apply_visitor_internal(d);
  }

  template <size_t index, typename... Args>
  void initialize(Args &&... args) {
    m_storage.template initialize<index>(std::forward<Args>(args)...);
    this->m_which = static_cast<int>(index);
  }

  /***
   * Used for internal visitors
   */
  template <typename Internal = detail::true_, typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) -> typename Visitor::result_type {
    return detail::visitor_dispatch<Internal, 1 + sizeof...(Types)>{}(m_which, m_storage, visitor);
  }

  template <typename Internal = detail::true_, typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) const -> typename Visitor::result_type {
    return detail::visitor_dispatch<Internal, 1 + sizeof...(Types)>{}(m_which, m_storage, visitor);
  }

  /***
   * find_which is used with non-T&& ctors to figure out what "which" should be
   * used for a given type
   */
  template <typename Rhs>
  struct find_which {
    static constexpr size_t value =
      mpl::Find_With<detail::same_modulo_const_ref_wrapper<Rhs>::template prop, First,
                     Types...>::value;
    static_assert(value < (sizeof...(Types) + 1), "No match for value");
  };

  /***
   * Visitors used to implement special member functions and such
   */
  struct copy_constructor {
    typedef void result_type;

    explicit copy_constructor(variant & self)
      : m_self(self) {}

    template <typename T>
    void operator()(const T & rhs) const {
      m_self.initialize<find_which<T>::value>(rhs);
    }

  private:
    variant & m_self;
  };

  struct move_constructor {
    typedef void result_type;

    explicit move_constructor(variant & self)
      : m_self(self) {}

    template <typename T>
    void operator()(T & rhs) const noexcept {
      m_self.initialize<find_which<T>::value>(std::move(rhs));
    }

  private:
    variant & m_self;
  };

#define STRICT_VARIANT_ASSERT_NOTHROW_MOVE_CTORS                                                   \
  static_assert(nothrow_move_ctors,                                                                \
                "All types in this variant must be nothrow move constructible or the variant "     \
                "cannot be assigned!");                                                            \
  static_assert(true, "")

  // copy assigner
  struct copy_assigner {
    typedef void result_type;

    STRICT_VARIANT_ASSERT_NOTHROW_MOVE_CTORS;

    explicit copy_assigner(variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    void operator()(const Rhs & rhs) const {

      constexpr size_t index = find_which<Rhs>::value;

      if (m_self.which() == m_rhs_which) {
        // the types are the same, so just assign into the lhs
        STRICT_VARIANT_ASSERT(m_rhs_which == index, "Bad access!");
        m_self.m_storage.template get_value<index>(detail::true_{}) = rhs;
      } else if (assume_copy_nothrow || noexcept(Rhs(rhs))) {
        // If copy ctor is no-throw (think integral types), this is the fastest
        // way
        m_self.destroy();
        m_self.initialize<index>(rhs);
      } else {
        // Copy ctor could throw, so do trial copy on the stack for safety and
        // move it...
        Rhs tmp(rhs);
        m_self.destroy();                         // nothrow
        m_self.initialize<index>(std::move(tmp)); // nothrow (please)
      }
    }

  private:
    variant & m_self;
    int m_rhs_which;
  };

  // move assigner
  struct move_assigner {
    typedef void result_type;

    STRICT_VARIANT_ASSERT_NOTHROW_MOVE_CTORS;

    explicit move_assigner(variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    void operator()(Rhs & rhs) const {

      constexpr size_t index = find_which<Rhs>::value;

      if (m_self.which() == m_rhs_which) {
        // the types are the same, so just assign into the lhs
        STRICT_VARIANT_ASSERT(m_rhs_which == index, "Bad access!");
        m_self.m_storage.template get_value<index>(detail::true_{}) = std::move(rhs);
      } else {
        m_self.destroy();                         // nothrow
        m_self.initialize<index>(std::move(rhs)); // nothrow (please)
      }
    }

  private:
    variant & m_self;
    int m_rhs_which;
  };

  // equality check
  struct eq_checker {
    typedef bool result_type;

    eq_checker(const variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    bool operator()(const Rhs & rhs) const {

      constexpr size_t index = find_which<Rhs>::value;

      if (m_self.which() == m_rhs_which) {
        // the types are the same, so use operator eq
        STRICT_VARIANT_ASSERT(m_rhs_which == index, "Bad access!");
        return m_self.m_storage.template get_value<index>(detail::false_{}) == rhs;
      } else {
        return false;
      }
    }

  private:
    const variant & m_self;
    int m_rhs_which;
  };

  // destroyer
  struct destroyer {
    typedef void result_type;

    template <typename T>
    void operator()(T & t) const noexcept {
      t.~T();
    }
  };

  // initializer. This is the overloaded function object used in T && ctor.
  // Here T should be the forwarding reference

  // Init helper finds the target type, the priority of the conversion, and
  // whether it was allowed.
  template <typename T, unsigned idx>
  struct init_helper {
    using type = unwrap_type_t<typename storage_t::template value_t<idx>>;

    using trait = typename detail::allow_variant_construction<type, T>;

    static constexpr bool value = trait::value;
    static constexpr int priority = trait::priority;

    static constexpr bool valid_at_priority(int p) {
      return value && p < priority;
    }
  };

  // Initializer base is (possibly) a function object
  // If construction is prohibited, then don't generate operator()
  template <typename T, unsigned idx, int priority, typename ENABLE = void>
  struct initializer_base;

  template <typename T, unsigned idx, int priority>
  struct initializer_base<T, idx, priority, mpl::enable_if_t<init_helper<T, idx>::valid_at_priority(priority)>> {
    using target_type = typename init_helper<T, idx>::type;

    template <typename V>
    void operator()(V && v, target_type val) noexcept(noexcept(std::forward<V>(std::declval<V>()).template initialize<idx>(std::declval<target_type>()))) {
      std::forward<V>(v).template initialize<idx>(std::move(val));
    }
  };

  // If not valid then don't generate a call operator
  template <typename T, unsigned idx, int priority>
  struct initializer_base<T, idx, priority, mpl::enable_if_t<!init_helper<T, idx>::valid_at_priority(priority)>> {};


  // Report problem
  template <typename T, unsigned idx>
  struct report_problem {
    // Force delayed instantiation
    template <typename U>
    constexpr report_problem(U &&) {
      // TODO: Clang seems to always instantiate these even when it shouldn't... not sure why
      // static_assert(std::is_constructible<typename init_helper<T, idx>::type, T>::value, "No construction is possible!");
      // static_assert(!std::is_constructible<typename init_helper<T, idx>::type, T>::value || init_helper<T, idx>::value, "Conversion wasn't permitted!");
      // static_assert(!std::is_constructible<typename init_helper<T, idx>::type, T>::value || !init_helper<T, idx>::value, "Not clear whats wrong!");
    }
  };

  // valid_property
  template <int p>
  struct valid_at {
    template <typename T>
    struct prop {
      static constexpr bool value = T::valid_at_priority(p);
    };
  };

  // Any_At_Priority
  template <typename T, int priority, unsigned ... us>
  struct Any_At_Priority {
    static constexpr bool value = mpl::Find_Any<valid_at<priority>::template prop, init_helper<T, us>...>::value;
  };

  // Test_Priority
  // Find the highest level at which we are able to construct the type.
  template <typename T, int priority, typename UL = mpl::count_t<sizeof...(Types) + 1>, typename Enable = void>
  struct Test_Priority;

  template <typename T, int priority, unsigned ... us>
  struct Test_Priority<T, priority, mpl::ulist<us...>, mpl::enable_if_t<priority >= 0 && Any_At_Priority<T, priority, us...>::value>> {
    static constexpr int value = priority;
  };

  template <typename T, int priority, unsigned ... us>
  struct Test_Priority<T, priority, mpl::ulist<us...>, mpl::enable_if_t<priority >= 0 && !Any_At_Priority<T, priority, us...>::value>> : Test_Priority<T, priority - 1, mpl::ulist<us...>> {};

  // Fire a bunch of static asserts explaining that we cannot construct the variant
  template <typename T, unsigned ... us>
  struct Test_Priority<T, -1, mpl::ulist<us...>> {
    static constexpr int dummy[] = { (report_problem<T, us>{0}, 0)..., 0 };
    static constexpr int value = -1;
  };

  // Main object, created using inheritance
  // T should be a forwarding reference
  template <typename T, typename UL = mpl::count_t<sizeof...(Types) + 1>>
  struct initializer;

  template <typename T, unsigned ... us>
  struct initializer<T, mpl::ulist<us...>> : initializer_base<T, us, Test_Priority<T, mpl::priority_max>::value>... {};


#define STRICT_VARIANT_ASSERT_WHICH_INVARIANT                                                      \
  STRICT_VARIANT_ASSERT(static_cast<unsigned>(this->which()) < sizeof...(Types) + 1,               \
                        "Postcondition failed!")


public:
  template <typename = void> // only allow if First() is ok
  variant() noexcept(noexcept(First())) {
    static_assert(std::is_same<void, decltype(static_cast<void>(First()))>::value,
                  "First type must be default constructible or variant is not!");
    this->initialize<0>();
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  }

  ~variant() noexcept { this->destroy(); }

  // Special member functions
  variant(const variant & rhs) noexcept(nothrow_copy_ctors) {
    copy_constructor c(*this);
    rhs.apply_visitor_internal(c);
    STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  }

  // Note: noexcept is enforced by static_assert in move_constructor visitor
  variant(variant && rhs) noexcept(nothrow_move_ctors) {
    move_constructor mc(*this);
    rhs.apply_visitor_internal(mc);
    STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  }

  variant & operator=(const variant & rhs) noexcept(nothrow_copy_assign) {
    if (this != &rhs) {
      copy_assigner a(*this, rhs.which());
      rhs.apply_visitor_internal(a);
      STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    }
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
    return *this;
  }

  // TODO: If all types are nothrow MA then this is also
  // For now we assume it is the case.
  variant & operator=(variant && rhs) noexcept(nothrow_move_assign) {
    if (this != &rhs) {
      move_assigner ma(*this, rhs.which());
      rhs.apply_visitor_internal(ma);
      STRICT_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    }
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
    return *this;
  }

  /// Forwarding-reference ctor, construct a variant from one of its value
  /// types.
  /// The details are in `detail::allow_variant_construction`.
  /// See documentation
  template <typename T, typename = mpl::enable_if_t<!std::is_same<variant &, mpl::remove_const_t<T>>::value>, typename = decltype((*static_cast<initializer<T>*>(nullptr))(*static_cast<variant*>(nullptr), std::forward<T>(std::declval<T>())), void())>
  variant(T && t) noexcept(noexcept((*static_cast<initializer<T>*>(nullptr))(*static_cast<variant*>(nullptr), std::forward<T>(std::declval<T>())))){
    static_assert(!std::is_same<variant &, mpl::remove_const_t<T>>::value,
                  "why is variant(T&&) instantiated with a variant? why was a special "
                  "member function not selected?");
    initializer<T> initer;
    initer(*this, std::forward<T>(t));
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  }

  /// Friend all other instances of variant (needed for next two ctors)
  template <typename F, typename... Ts>
  friend class variant;

  /// "Generalizing Ctor"
  /// Allow constructing from a variant over a subset of our types
  /// (Boost variant does this, and we need it to comfortably interact with
  /// spirit)
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant(const variant<OFirst, OTypes...> & other) noexcept(
    variant<OFirst, OTypes...>::nothrow_copy_ctors) {
    copy_constructor c(*this);
    other.apply_visitor_internal(c);
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  }

  /// "Generalizing" move ctor, similar as above
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant(variant<OFirst, OTypes...> && other) noexcept(
    variant<OFirst, OTypes...>::nothrow_move_ctors) {
    move_constructor c(*this);
    other.apply_visitor_internal(c);
    STRICT_VARIANT_ASSERT_WHICH_INVARIANT;
  }

  // Emplace ctor. Used to explicitly specify the type of the variant, and
  // invoke an arbitrary ctor of that type.
  template <typename T>
  struct emplace_tag {};

  template <typename T, typename... Args>
  explicit variant(emplace_tag<T>,
                   Args &&... args) noexcept(std::is_nothrow_constructible<T, Args...>::value) {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

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
  template <typename T, typename... Args>
  mpl::enable_if_t<!std::is_nothrow_constructible<T, Args...>::value> // returns void
    emplace(Args &&... args) noexcept(false) {
    static_assert(std::is_nothrow_move_constructible<T>::value,
                  "To use emplace, either the invoked ctor or the move ctor must be noexcept.");
    T temp(std::forward<Args>(args)...);
    this->emplace<T>(std::move(temp));
  }

  template <typename T, typename... Args>
  mpl::enable_if_t<std::is_nothrow_constructible<T, Args...>::value> // returns void
    emplace(Args &&... args) noexcept {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    this->destroy();
    this->initialize<idx>(std::forward<Args>(args)...);
  }

  /***
   * Accessors
   */

  int which() const noexcept { return m_which; }

  // operator ==
  bool operator==(const variant & rhs) const {
    eq_checker eq(*this, rhs.which());
    // Pass detail::false because it needs to pierce the recursive wrapper
    return rhs.apply_visitor_internal<detail::false_>(eq);
  }

  bool operator!=(const variant & rhs) const { return !(*this == rhs); }

  // get
  template <typename T>
  T * get() noexcept {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    if (idx == m_which) {
      return &m_storage.template get_value<idx>(detail::false_{});
    } else {
      return nullptr;
    }
  }

  template <typename T>
  const T * get() const noexcept {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    if (idx == m_which) {
      return &m_storage.template get_value<idx>(detail::false_{});
    } else {
      return nullptr;
    }
  }

  // Friend apply_visitor
  template <typename Visitor, typename Visitable>
  friend auto apply_visitor(Visitor &&, Visitable &&)
    -> decltype(std::declval<Visitable>().get_visitor_dispatch()(
      std::declval<Visitable>().which(),
      std::forward<Visitable>(std::declval<Visitable>()).storage(),
      std::forward<Visitor>(std::declval<Visitor>())));

  // Implementation details for apply_visitor
private:
  storage_t & storage() & { return m_storage; }
  storage_t && storage() && { return std::move(m_storage); }
  const storage_t & storage() const & { return m_storage; }

  detail::visitor_dispatch<detail::false_, 1 + sizeof...(Types)> get_visitor_dispatch() {
    return {};
  }
  detail::visitor_dispatch<detail::false_, 1 + sizeof...(Types)> get_visitor_dispatch() const {
    return {};
  }
};

/***
 * apply one visitor function. This is the basic version, used in implementation
 * of multivisitation.
 */
template <typename Visitor, typename Visitable>
auto
apply_visitor(Visitor && visitor, Visitable && visitable)
  -> decltype(std::declval<Visitable>().get_visitor_dispatch()(
    std::declval<Visitable>().which(), std::forward<Visitable>(std::declval<Visitable>()).storage(),
    std::forward<Visitor>(std::declval<Visitor>()))) {
  return visitable.get_visitor_dispatch()(visitable.which(),
                                          std::forward<Visitable>(visitable).storage(),
                                          std::forward<Visitor>(visitor));
}

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

// Helper for generic programming -- produce a variant, wrapping each type which
// has a throwing move in `recursive_wrapper`.
template <typename... Ts>
using easy_variant = variant<wrap_if_throwing_move_t<Ts>...>;

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
#undef STRICT_VARIANT_ASSERT_WHICH_INVARIANT
#undef STRICT_VARIANT_ASSERT_NOTHROW_MOVE_CTORS
