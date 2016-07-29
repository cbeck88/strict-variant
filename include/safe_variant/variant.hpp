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
 * time and can be hard to follow.
 *
 * User beware -- in this project, we have mostly banned exceptions from the
 * code-base, and this variant has been configured to assume that it's
 * underlying types don't throw generally, as an optimisation.
 * If you are using it in another project, you may want to turn off the
 * "assume_copy_nothrow" flag below.
 *
 * At minimum, any type used with this should not throw exceptions from dtor,
 * or from move ctor, or from move assignment, regardless of that flag.
 *
 * If one of the types does throw in these situations, generally it will cause a
 * call to std::terminate due to noexcept specification
 *
 * This code is derived from (an early version of) Jarryd Beck's variant:
 *   https://github.com/jarro2783/thenewcpp
 *
 */

#include <safe_variant/recursive_wrapper.hpp>
#include <safe_variant/static_visitor.hpp>
#include <safe_variant/variant_detail.hpp>
#include <safe_variant/variant_fwd.hpp>

#include <safe_variant/find_with.hpp>
#include <safe_variant/index.hpp>
#include <safe_variant/safely_constructible.hpp>
#include <safe_variant/std_traits.hpp>
#include <safe_variant/typelist.hpp>

#include <new>
#include <type_traits>
#include <utility>

// #define SAFE_VARIANT_DEBUG

#ifdef SAFE_VARIANT_DEBUG
#include <cassert>

#define SAFE_VARIANT_ASSERT(X, C)                                                                  \
  do {                                                                                             \
    assert((X) && C);                                                                              \
  } while (0)

#else

#define SAFE_VARIANT_ASSERT(X, C)                                                                  \
  do {                                                                                             \
  } while (0)

#endif

namespace safe_variant {

/***
 * Class variant
 */
template <typename First, typename... Types>
class variant {

private:
  /***
   * TODO: Prohibit const and reference types
   */

  /***
   * Check noexcept status of special member functions of our types
   */

  static_assert(mpl::All_Have<std::is_nothrow_destructible, First>::value,
                "All types in this variant type must be nothrow destructible");
  static_assert(mpl::All_Have<std::is_nothrow_destructible, Types...>::value,
                "All types in this variant type must be nothrow destructible");

  // static_assert(mpl::All_Have<std::is_nothrow_move_constructible,
  // First>::value,
  //               "All types in this variant type must be nothrow move
  //               constructible");
  // static_assert(mpl::All_Have<std::is_nothrow_move_constructible,
  // Types...>::value,
  //               "All types in this variant type must be nothrow move
  //               constructible");

  /// Detect what happens for copy ctor, copy assignment
  // static constexpr bool noexcept_copy_operations =
  // //mpl::All_Have<std::nothrow_copy_assignable,
  // First, Types...>::value &&
  //           mpl::All_Have<std::is_nothrow_copy_constructible, First,
  //           Types...>::value;

  // This "assume_copy_nothrow" assumption allows us to go faster in cases of
  // e.g. copy assign from
  // std::string, since we avoid an unnecessary "safety copy on the stack". With
  // the downside that,
  // if we run out of memory and std::string copy ctor throws, we lose strong
  // exception safey, but
  // in this project we will crash in that case anyways. Not appropriate for
  // fighter-jet code :p
  static constexpr bool assume_copy_nothrow = false;

  /***
   * Determine size and alignment of our storage
   */
  template <typename T>
  struct Sizeof {
    static constexpr size_t value = sizeof(T);
  };

  template <typename T>
  struct Alignof {
    static constexpr size_t value = alignof(T);
  };

  // size = max of size of each thing
  static constexpr size_t m_size = mpl::max<Sizeof, First, Types...>::value;

  // align = max align of each thing
  static constexpr size_t m_align = mpl::max<Alignof, First, Types...>::value;

  /***
   * Storage
   */

  alignas(m_align) char m_storage[m_size];

  int m_which;

  void * address() { return m_storage; }
  const void * address() const { return m_storage; }

  /***
   * Initialize and destroy
   */
  void destroy() {
    destroyer d;
    this->apply_visitor_internal(d);
  }

  template <size_t index, typename... Args>
  void initialize(Args && ... args) {
    using target_type = mpl::Index_t<index, First, Types...>;
    new (m_storage) target_type(std::forward<Args>(args)...);
    this->m_which = static_cast<int>(index);
  }

  /***
   * Unchecked typed access to storage
   */
  template <size_t index>
  mpl::Index_t<index, First, Types...> * unchecked_access() {
    return reinterpret_cast<mpl::Index_t<index, First, Types...> *>(this->address());
  }

  template <size_t index>
  const mpl::Index_t<index, First, Types...> * unchecked_access() const {
    return reinterpret_cast<const mpl::Index_t<index, First, Types...> *>(this->address());
  }

  /***
   * Used for internal visitors
   */
  template <typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) -> typename Visitor::result_type {
    return this->apply_visitor<detail::true_>(visitor);
  }

  template <typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) const -> typename Visitor::result_type {
    return this->apply_visitor<detail::true_>(visitor);
  }

  /***
   * find_which is used with non-T&& ctors to figure out what "which" should be
   * used for a given type
   */

  // Search prediate for find_which
  template <typename T>
  struct same_modulo_const_ref_wrapper {
    template <typename U>
    struct prop {
      using T2 = unwrap_type_t<mpl::remove_const_t<mpl::remove_reference_t<T>>>;
      using U2 = unwrap_type_t<mpl::remove_const_t<mpl::remove_reference_t<U>>>;

      static constexpr bool value = std::is_same<T2, U2>::value;
    };
  };

  template <typename Rhs>
  struct find_which {
    static constexpr size_t value =
              mpl::Find_With<same_modulo_const_ref_wrapper<Rhs>::template prop, First, Types...>::value;
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

  // copy assigner
  struct copy_assigner {
    typedef void result_type;

    explicit copy_assigner(variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    void operator()(const Rhs & rhs) const {

      constexpr size_t index = find_which<Rhs>::value;

      if (m_self.which() == m_rhs_which) {
        // the types are the same, so just assign into the lhs
        SAFE_VARIANT_ASSERT(m_rhs_which == index, "Bad access!");
        (*m_self.unchecked_access<index>()) = rhs;
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

    explicit move_assigner(variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    void operator()(Rhs & rhs) const {

      constexpr size_t index = find_which<Rhs>::value;

      if (m_self.which() == m_rhs_which) {
        // the types are the same, so just assign into the lhs
        SAFE_VARIANT_ASSERT(m_rhs_which == index, "Bad access!");
        (*m_self.unchecked_access<index>()) = std::move(rhs);
      } else {
        m_self.destroy();                 // nothrow
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
        SAFE_VARIANT_ASSERT(m_rhs_which == index, "Bad access!");
        return (*m_self.unchecked_access<index>()) == rhs;
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

  #define SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT                                   \
    SAFE_VARIANT_ASSERT(static_cast<unsigned>(this->which()) < sizeof...(Types) + 1, "Postcondition failed!")

public:
  template <typename = void> // only allow if First() is ok
  variant() {
    static_assert(std::is_same<void, decltype(static_cast<void>(First()))>::value,
                  "First type must be default constructible or variant is not!");
    this->initialize<0>();
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
  }

  ~variant() noexcept { this->destroy(); }

  // Special member functions
  variant(const variant & rhs) noexcept(assume_copy_nothrow) {
    copy_constructor c(*this);
    rhs.apply_visitor_internal(c);
    SAFE_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
  }

  variant(variant && rhs) noexcept {
    move_constructor mc(*this);
    rhs.apply_visitor_internal(mc);
    SAFE_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
  }

  variant & operator=(const variant & rhs) noexcept(assume_copy_nothrow) {
    if (this != &rhs) {
      copy_assigner a(*this, rhs.which());
      rhs.apply_visitor_internal(a);
      SAFE_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    }
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
    return *this;
  }

  // TODO: If all types are nothrow MA then this is also
  // For now we assume it is the case.
  variant & operator=(variant && rhs) noexcept {
    if (this != &rhs) {
      move_assigner ma(*this, rhs.which());
      rhs.apply_visitor_internal(ma);
      SAFE_VARIANT_ASSERT(rhs.which() == this->which(), "Postcondition failed!");
    }
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
    return *this;
  }

  /// Forwarding-reference ctor, construct a variant from one of its value
  /// types.
  /// The details are in `detail::allow_variant_construction`.
  /// See documentation
  template <typename T,
            typename Enable =
              mpl::enable_if_t<mpl::Find_Any<detail::allow_variant_construct_from<T>::template prop,
                                             First, Types...>::value>>
  variant(T && t) {
    static_assert(!std::is_same<variant &, mpl::remove_const_t<T>>::value,
                  "why is variant(T&&) instantiated with a variant? why was a special "
                  "member function not selected?");
    constexpr size_t which_idx =
      mpl::Find_With<detail::allow_variant_construct_from<T>::template prop, First,
                     Types...>::value;
    static_assert(which_idx < (sizeof...(Types) + 1), "Could not construct variant from this type!");
    this->initialize<which_idx>(std::forward<T>(t));
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
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
  variant(const variant<OFirst, OTypes...> & other) {
    copy_constructor c(*this);
    other.apply_visitor_internal(c);
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
  }

  /// "Generalizing" move ctor, similar as above
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant(variant<OFirst, OTypes...> && other) noexcept {
    move_constructor c(*this);
    other.apply_visitor_internal(c);
    SAFE_VARIANT_CTOR_POSTCONDITION_ASSERT;
  }

  /***
   * operator ==
   */

  bool operator==(const variant & rhs) const {
    eq_checker eq(*this, rhs.which());
    return rhs.apply_visitor_internal(eq);
  }

  bool operator!=(const variant & rhs) const { return !(*this == rhs); }

  // Access which()
  int which() const { return m_which; }

  // Apply visitor
  template <typename Internal, typename Visitor, typename... Args>
  auto apply_visitor(Visitor && visitor, Args &&... args) -> vis_result_t<Visitor> {
    return detail::visitor_dispatch<First, Types...>()(
      Internal(), m_which, m_storage, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
  }

  template <typename Internal, typename Visitor, typename... Args>
  auto apply_visitor(Visitor && visitor, Args &&... args) const -> vis_result_t<Visitor> {
    return detail::visitor_dispatch<First, Types...>()(
      Internal(), m_which, m_storage, std::forward<Visitor>(visitor), std::forward<Args>(args)...);
  }

  // get
  template <typename T>
  T * get() {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    if (idx == m_which) {
      return &maybe_pierce_recursive_wrapper<T>(
        *this->unchecked_access<idx>());
    } else {
      return nullptr;
    }
  }

  template <typename T>
  const T * get() const {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    if (idx == m_which) {
      return &maybe_pierce_recursive_wrapper<T>(
        *this->unchecked_access<idx>());
    } else {
      return nullptr;
    }
  }

  // Emplace operation
  // In this operation the user explicitly specifies the desired type as
  // template parameter, which
  // must be one of the variant types, modulo const and reference wrapper.
  // We always destroy and reconstruct in-place.

  template <typename T, typename... Us>
  void emplace(Us &&... us) {
    constexpr size_t idx = find_which<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");

    this->destroy();
    this->initialize<idx>(std::forward<Us>(us)...);
  }
};

/***
 * apply visitor function (same semantics as safe_variant::apply_visitor)
 */
template <typename Visitor, typename Visitable, typename... Args>
auto
apply_visitor(Visitor && visitor, Visitable & visitable, Args &&... args) -> vis_result_t<Visitor> {
  return visitable.template apply_visitor<detail::false_>(std::forward<Visitor>(visitor),
                                                          std::forward<Args>(args)...);
}

/***
 * safe_variant::get function (same semantics as boost::get with pointer type)
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
get_or_default(variant<Types...> & v, T def = {}) {
  T * t = safe_variant::get<T>(&v);
  if (!t) {
    // v.template emplace<T>(std::move(def));
    v = std::move(def);
    t = safe_variant::get<T>(&v);
    SAFE_VARIANT_ASSERT(t, "Move assignment to a variant failed to change its type!");
  }
  return *t;
}

} // end namespace safe_variant
