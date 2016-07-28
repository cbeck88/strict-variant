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
 code-base,
 * and this variant has been configured to assume that it's underlying types
 don't
 * throw generally, as an optimisation.
 * If you are using it in another project, you may want to turn off the
 "assume_copy_nothrow"
 * flag below.
 *
 * At minimum, any type used with this should not throw exceptions from dtor,
 * or from move ctor, or from move assignment, regardless of that flag.
 *
 * If one of the types does throw in these situations, generally it will cause a
 call
 * to std::terminate due to member functions of variant being marked noexcept.
 *
 * This version is loosely based on (an early version of) Jarryd Beck's variant:
 *   https://github.com/jarro2783/thenewcpp
 *
 * There have been some significant changes:
 *
 * - Get rid of exception type, delete anything that throws
 * - Update storage type to use alignas rather than a union
 * - Add static_assert that all types involved are no-throw destructible.
 * - Use our assertions rather than cassert
 * - Mark some of the special member functions as noexcept, as appropriate.
 * - Allow to construct a variant from another variant on fewer types, in the
 natural way
 *   that you would expect.
 * - When variant is initialized from a given value, the selected internal type
 is now
 *   chosen in a special manner and NOT via overload resolution.
 *   Construction is enabled by a trait "allow_variant_construction<T, U>" which
 checks
 *   if T is constructible from U, but in case that both T and U are fundamental
 types
 *   or references to fundamental types, or are pointers, we only allow certain
 "safe"
 *   conversions defined by the mpl::safely_constructible trait.
 *   If construction is allowed for any of the internal types of the variant, we
 use the
 *   FIRST one in the list, rather than declaring ambiguity. This system,
 instead of
 *   creating a function object with an overload for each type and using
 overload
 *   resolution, is a bit simpler to reason about and makes it easier to achieve
 exactly
 *   the behavior you want in our use-cases.
 * - Add an operator ==
 * - Add operator << with ostreams
 * - Add a constructor which maps a "smaller" variant to a larger variant
     (Needed for full boost::variant compat / spirit usage)
 * - TODO: Add assignment operator for "smaller" variant to larger variant?
 * - TODO: Use binary tree comparison instead of manual jump table for visitor
 impl?
 *         Perhaps only below a certain cardinality?
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

namespace safe_variant {

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
   * Visitors used to implement special member functions and such
   */
  struct constructor {
    typedef void result_type;

    constructor(variant & self)
      : m_self(self) {}

    template <typename T>
    void operator()(const T & rhs) const {
      m_self.construct(rhs);
    }

  private:
    variant & m_self;
  };

  struct move_constructor {
    typedef void result_type;

    move_constructor(variant & self)
      : m_self(self) {}

    template <typename T>
    void operator()(T & rhs) const noexcept {
      m_self.construct(std::move(rhs));
    }

  private:
    variant & m_self;
  };

  // copy assigner
  struct assigner {
    typedef void result_type;

    assigner(variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    void operator()(const Rhs & rhs) const {

      if (m_self.which() == m_rhs_which) {
        // the types are the same, so just assign into the lhs
        *reinterpret_cast<Rhs *>(m_self.address()) = rhs;
      } else if (assume_copy_nothrow || noexcept(Rhs(rhs))) {
        // If copy ctor is no-throw (think integral types), this is the fastest
        // way
        m_self.destroy();
        m_self.construct(rhs);
      } else {
        // Copy ctor could throw, so do trial copy on the stack for safety and
        // move it...
        Rhs tmp(rhs);
        m_self.destroy();                 // nothrow
        m_self.construct(std::move(tmp)); // nothrow (please)
      }
    }

  private:
    variant & m_self;
    int m_rhs_which;
  };

  // move assigner
  struct move_assigner {
    typedef void result_type;

    move_assigner(variant & self, int rhs_which)
      : m_self(self)
      , m_rhs_which(rhs_which) {}

    template <typename Rhs>
    void operator()(Rhs & rhs) const {
      using RhsNoConst = mpl::remove_const_t<Rhs>;
      if (m_self.which() == m_rhs_which) {
        // the types are the same, so just assign into the lhs
        *reinterpret_cast<RhsNoConst *>(m_self.address()) = std::move(rhs);
      } else {
        m_self.destroy();                 // nothrow
        m_self.construct(std::move(rhs)); // nothrow (please)
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
      if (m_self.which() == m_rhs_which) {
        // the types are the same, so use operator eq
        return *reinterpret_cast<const Rhs *>(m_self.address()) == rhs;
      } else {
        return false;
      }
    }

  private:
    const variant & m_self;
    int m_rhs_which;
  };

  // whicher
  struct whicher {
    typedef int result_type;

    template <typename Rhs>
    constexpr int operator()(const Rhs &) const noexcept {
      return static_cast<int>(
        mpl::Find_With<mpl::sameness<Rhs>::template prop, First, Types...>::value);
    }
  };

  // Destructor
  struct destroyer {
    typedef void result_type;

    template <typename T>
    void operator()(T & t) const noexcept {
      t.~T();
    }
  };

  // Initialiser
  template <size_t which>
  struct initialiser {
    template <typename T>
    static void initialise(variant & v, T && arg) {
      do_init(v, std::forward<T>(arg));
    }

    using target_type = mpl::Index_t<which, First, Types...>;

    static void do_init(variant & v, target_type && arg) {
      v.construct(std::move(arg));
      v.indicate_which(which);
    }

    static void do_init(variant & v, const target_type & arg) {
      v.construct(arg);
      v.indicate_which(which);
    }
  };

public:
  template <typename = void> // force delayed instantiation
  variant() {
    // try to construct First
    // if this fails then First is not default constructible
    this->construct(First());
    this->indicate_which(0);
  }

  ~variant() noexcept { destroy(); }

  /// Forwarding-reference ctor, construct a variant from one of its value
  /// types.
  /// If given type T can be used to construct any of our types, then we want
  /// to *pick the first eligible one* and use it. This is what initialise does.
  ///
  /// Note: This ctor allows the use of user-defined conversions and converting
  /// ctors, for example, if the variant contains `std::string` and you pass
  /// this
  /// ctor a `const char *`, it will construct a `std::string` using that ctor
  /// of
  /// `std::string`.
  ///
  /// Ambiguities are always resolved by preferring the type that occurs earlier
  /// in the list, so the order in which types appear in the variant is very
  /// significant towards how it behaves!
  /// E.g. if you have variant<std::string, const char *>, constructing this
  /// from
  /// const char * will produce a `std::string`, while if it is
  /// `variant<const char *, std::string>` it will produce a const char *.
  ///
  /// typedef variant<std::string, const char *> Var_t;
  /// Var_t a{"asdf"};  // Contains a std::string
  ///
  /// typedef variant<const char *, std::string> Var_t;
  /// Var_t a{"asdf"};  // Contains a const char *
  ///
  ///
  /// Note that this is in CONTRAST to how C++ function overload resolution
  /// works,
  /// where the order of declaration doesn't matter and constructions may be
  /// declared ambiguous!
  ///
  /// Additionally, we prohibit many "unsafe" integral conversions.
  /// - No conversion from integral <-> floating point
  /// - No conversion from bool <-> integral
  /// - No conversion unsigned -> signed
  /// - Signed -> Unsigned is allowed only if they are otherwise the same type.
  /// - No conversion which would cause truncation.
  ///
  /// When making variant a which contains integral types, you should usually
  /// declare them in increasing order of size. E.g.:
  ///
  /// typedef variant<double, float, int> Var_t
  /// Var_t a{5};       // Contains an int
  /// Var_t b{10.0f};   // Contains a double :(
  /// Var_t c{10.0};    // Contains a double
  ///
  /// typedef variant<int, float, double> Var_t2;
  /// Var_t2 a{5};      // Contains an int
  /// Var_t2 b{10.0f};  // Contains a float   :)
  /// Var_t2 c{10.0};   // Contains a double
  ///
  /// You do not generally need to use a "safe_bool" type instead of a bool,
  /// because the boolean conversions are blocked by the "safely_constructible"
  /// mechanism.
  ///
  /// typedef variant<bool, int, float> Var_t3;
  /// Var_t3 a{true};  // Contains a bool
  /// Var_t3 b{1};     // Contains an int
  /// Var_t3 c{10.0f};   // Contains a float
  ///
  /// typedef variant<int, bool, float> Var_t4;
  /// Var_t4 a{true};  // Contains an bool
  /// Var_t4 b{1};     // Contains an int
  /// Var_t4 c{10.0f};   // Contains a float
  ///
  /// Implementation note:
  /// An additional issue to consider is what happens when the variant is
  /// declared
  /// using an incomplete type, like safe_variant::recursive_wrapper<T>
  ///
  /// Generally, what we do is allow construction in that slot from T, const T
  /// &,
  /// and T&&, as well as well as safe_variant::recursive_wrapper<T>,
  /// const safe_variant::recursive_wrapper<T> &, and safe_variant::recursive_wrapper<T> &&.
  /// This check can be performed without knowing the complete defintiion of T,
  /// so even if there are "point of instantiation" issues it will work out.
  /// The details are in `detail::allow_variant_construction`.
  ///
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
    initialiser<which_idx>::initialise(*this, std::forward<T>(t));
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
    whicher w; // construct whicher for MY type
    // get the which before applying the other visitor
    int new_which = other.apply_visitor_internal(w);

    constructor c(*this);
    other.apply_visitor_internal(c);
    this->indicate_which(new_which);
  }

  /// "Generalizing" move ctor, similar as above
  template <typename OFirst, typename... OTypes,
            typename Enable = mpl::enable_if_t<detail::proper_subvariant<variant<OFirst, OTypes...>,
                                                                         variant>::value>>
  variant(variant<OFirst, OTypes...> && other) noexcept {
    whicher w; // construct whicher for MY type
    // get which of other which before applying the other visitor
    int new_which = other.apply_visitor_internal(w);

    move_constructor c(*this);
    other.apply_visitor_internal(c);
    this->indicate_which(new_which);
  }

  variant(const variant & rhs) noexcept(assume_copy_nothrow) {
    constructor c(*this);
    rhs.apply_visitor_internal(c);
    this->indicate_which(rhs.which());
  }

  variant(variant && rhs) noexcept {
    move_constructor mc(*this);
    rhs.apply_visitor_internal(mc);
    this->indicate_which(rhs.which());
  }

  variant & operator=(const variant & rhs) noexcept(assume_copy_nothrow) {
    if (this != &rhs) {
      assigner a(*this, rhs.which());
      rhs.apply_visitor_internal(a);
      this->indicate_which(rhs.which());
    }
    return *this;
  }

  // TODO: If all types are nothrow MA then this is also
  // For now we assume it is the case.
  variant & operator=(variant && rhs) noexcept {
    if (this != &rhs) {
      move_assigner ma(*this, rhs.which());
      rhs.apply_visitor_internal(ma);
      this->indicate_which(rhs.which());
    }
    return *this;
  }

  bool operator==(const variant & rhs) const {
    eq_checker eq(*this, rhs.which());
    return rhs.apply_visitor_internal(eq);
  }

  bool operator!=(const variant & rhs) const { return !(*this == rhs); }

  int which() const { return m_which; }

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

  // Helper template which, given a "get" request, gets the index of the type in
  // our list which
  // corresponds to it. It must match exactly, modulo const and
  // recursive_wrapper.
  template <typename T>
  struct get_index_helper {
    static constexpr size_t value =
      mpl::Find_With<mpl::sameness<mpl::remove_const_t<T>>::template prop, unwrap_type_t<First>,
                     unwrap_type_t<Types>...>::value;
  };

  template <typename T>
  T * get() {
    constexpr size_t idx = get_index_helper<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");
    using internal_type = mpl::Index_t<idx, First, Types...>;

    if (idx == m_which) {
      return &maybe_pierce_recursive_wrapper<T>(
        *reinterpret_cast<internal_type *>(this->address()));
    } else {
      return nullptr;
    }
  }

  template <typename T>
  const T * get() const {
    constexpr size_t idx = get_index_helper<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");
    using internal_type = mpl::Index_t<idx, First, Types...>;

    if (idx == m_which) {
      return &maybe_pierce_recursive_wrapper<T>(
        *reinterpret_cast<const internal_type *>(this->address()));
    } else {
      return nullptr;
    }
  }

  // Emplace operation
  // In this operation the user explicitly specifies the desired type as
  // template parameter, which
  // must be one of the variant types, modulo const and reference wrapper.
  // We always destroy and reconstruct in-place.
  template <typename T>
  struct emplace_index_helper {
    static constexpr size_t value =
      mpl::Find_With<mpl::sameness<mpl::remove_const_t<T>>::template prop, unwrap_type_t<First>,
                     unwrap_type_t<Types>...>::value;
  };

  template <typename T, typename... Us>
  void emplace(Us &&... us) {
    constexpr size_t idx = emplace_index_helper<T>::value;
    static_assert(idx < sizeof...(Types) + 1,
                  "Requested type is not a member of this variant type");
    using internal_type = mpl::Index_t<idx, First, Types...>;

    this->destroy();
    new (m_storage) internal_type(std::forward<Us>(us)...);
    this->indicate_which(idx);
  }

private:
  alignas(m_align) char m_storage[m_size];

  int m_which;

  void indicate_which(int which) { m_which = which; }

  void * address() { return m_storage; }
  const void * address() const { return m_storage; }

  template <typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) -> typename Visitor::result_type {
    return this->apply_visitor<detail::true_>(visitor);
  }

  template <typename Visitor>
  auto apply_visitor_internal(Visitor & visitor) const -> typename Visitor::result_type {
    return this->apply_visitor<detail::true_>(visitor);
  }

  void destroy() {
    destroyer d;
    this->apply_visitor_internal(d);
  }

  template <typename T>
  void construct(T && t) {
    using type = mpl::remove_reference_t<T>;
    new (m_storage) type(std::forward<T>(t));
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
  if (T * t = safe_variant::get<T>(&v)) { return *t; }
  v = std::move(def);
  T * t = safe_variant::get<T>(&v);
  // ASSERT(t && "Move assignment to a variant failed to change its type!");
  return *t;
}

} // end namespace safe_variant
