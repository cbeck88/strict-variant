# safe variant

[![Build Status](https://travis-ci.org/cbeck88/safe-variant.svg?branch=master)](http://travis-ci.org/cbeck88/safe-variant)
[![Coverage Status](https://coveralls.io/repos/cbeck88/safe-variant/badge.svg?branch=master&service=github)](https://coveralls.io/github/cbeck88/safe-variant?branch=master)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

Do you use `boost::variant` or one of the many open-source C++11 implementations of a "tagged union" or variant type
in your C++ projects?

Do you get annoyed that code like this will compile, without any warning or error message?

```c++
  boost::variant<std::string, int> v;  

  v = true;  
```

Do you get annoyed that code like this will compile on some machines, but not others?

```c++
  boost::variant<long, unsigned int> v;  

  v = 10;  
```


If so, then this may be the variant type for you.

**safe variant** is yet another C++11 variant type, with the twist that it prevents "unsafe" implicit conversions
such as narrowing conversions, conversions from bool to other integral types, pointer conversions, etc., and handles
overload ambiguity differently from other C++ variant types.

It may be well-suited for use in scenarios where you need to have a variant holding multiple different integral types,
and really don't want to have any loss of precision or any "gotcha" conversions happening.

Overview
========

The reason that `boost::variant` and most other variants, including the `std::variant` which was accepted to C++17,
will allow implicit conversions, is that fundamentally they work through C++ overload resolution.

Generally, when you assign a value of some type `T` to such a variant, these variants are going to construct a temporary function object
which is overloaded once for each type in the variant's list. Then they apply the function object to the value, and overload
resolution selects which conversion will actually happen.

Overload resolution is a core C++ language feature, and in 95% of cases it works very well and does the right thing.

However, in the 5% of cases where overload resolution does the wrong thing, it can be quite difficult to work around it.
This includes the scenarios in which overload resolution is ambiguous, as well as the cases in which, due to some implicit conversion,
an overload is selected which the user did not intend.

Because integral types have so many permitted conversions, these problems are particularly obvious when you have a variant with
several integral types.

This happens commonly when using variant types to interface with some scripting language for instance. The typical dynamically-typed
scripting language will permit a variety of primitive values, so when binding to it, you may naturally end up with something like

```c++
    boost::variant<bool, int, float, std::string, ...>
```

**safe variant** therefore does not use overload resolution in its implementation.  

Instead, it uses a very simple iterative strategy.

- When the variant is constructed from a value, each type is checked one by one, to see if a *safe* conversion to that type is possible.
  If so, it is selected. If not, we check the next type. If no safe conversion is possible, then a compile-time error results.  
  This means that usually, when you declare your variants you simply list your integral types in "increasing" order, and it does the right thing.

- What conversions are safe?  
  I wrote a type trait that implements a strict notion of safety which was appropriate for the project in which
  I developed this. (See [1](include/safe_variant/conversion_rank.hpp), [2](include/safe_variant/safely_constructible.hpp)).
  - Conversions are not permitted between any two of the following classes:  
  Integral types, Floating point types, Character types, Boolean, Pointer types, and `wchar_t`.
  - If an integral or floating point conversion *could* be narrowing on some conforming implementation of C++, then it is not safe.  
  (So, `long` cannot be converted to `int`
  even if you are on a 32-bit machine and they have the same size for you, because it could be narrowing on a 64-bit machine.)
  - Signed can be promoted to unsigned, but the reverse is not allowed (since it is implementation-defined).
  - Conversions like `char *` to `const char *` are permitted, and standard conversions like array-to-pointer are permitted, but otherwise no pointer conversions are permitted.

- You can force the variant to a particular type using the `emplace` template function. Rarely necessary in my experience but sometimes useful, and saves a `move`.
- There is also an emplace-ctor, where you select the type using tag dispatch.

  ```c++
    variant v{variant::emplace_tag<my_type>, arg1, arg2, arg3};
  ```

So, keep in mind, this is not a drop-in replacement for `boost::variant` or one of the other versions, its semantics are fundamentally different.
But in scenarios like those it was designed for, it may be easier to reason about and less error-prone.

Assumptions
===========

- Any type in the variant must be no-throw destructible.
- References are not allowed, use `std::reference_wrapper`.

Never Empty Guarantee
=====================

We deal with the "never empty" issue as follows:

**Every type used with the variant must be no-throw move constructible, or the variant is not assignable.**

This is enforced using static asserts within the assignment operators. The condition can sometimes be a pain if you are forced to use e.g. GCC 4-series versions of the C++ standard library which
are not C++11 conforming, and not all move ctors are appropriately marked `noexcept`. So there is also a flag to turn the static asserts off, see configuration for details. (Note that if a move does throw in this case, you will get UB.)

This allows the implementation to be very simple and efficient compared with some other variant types, which may have to make extra copies to facilitate
exception-safety, or make only a "rarely empty" guarantee.

recursive wrapper
-----------------

*If you have a type `T` with a throwing move*, you are encouraged to use `safe_variant::recursive_wrapper<T>` instead of `T` in the variant.

`recursive_wrapper<T>` is behaviorally equivalent to `std::unique_ptr<T>`, making a copy of `T` on the heap. But it is convertible to `T&`, and so
for most purposes is syntactically the same as `T`. There is special support within `safe_variant::variant` so that you can call `get<T>(&v)` and get a pointer
to a `T` rather than the wrapper, for instance, and similarly throughout the `variant` interface.

`recursive_wrapper<T>` always has a `noexcept` move ctor even if `T` does not.

By mostly restricting attention to no-throw move constructible types, the guts of the variant enjoy a very clean and simple implementation
-- there are no dynamic allocations taking place behind your back, and you mostly "intuitively" know what the variant is doing and what the costs
are when you manipulate it. Moving the variant won't be inherently more expensive than moving an individual object, and when copying, which let's imagine
can throw, something similar to "copy and swap" (or rather "copy and move") takes place, with a single copy occuring on the stack before we move it into
storage.

If you want additional dynamic allocations beyond this in order to support throwing moves, you opt-in to that using `recursive_wrapper`, and otherwise you don't pay for
what you don't use. There's no other additional storage in the variant or complexity added to the interface.

Note that the *stated* purpose of recursive wrapper, in `boost::variant` docs, is to allow you to declare variants which contain an incomplete type.
It also works great for that in `safe_variant::variant`.

emplace
-------

Even if the variant is not assignable, you can still use the `emplace` function to change the type of its value, provided that the constructor you invoke is `noexcept` or the requested type is no-throw move constructible.

easy variant
-------------

Additionally, we provide a template `easy_variant` which takes care of these details if you don't care to be bothered by the compiler about a throwing move / dynamic allocation.  

(Some programmers would prefer that the compiler not start making dynamic allocations without a warning though, just because some `noexcept` annotation was not deduced the way they expected.)  

Specifically, any type that you put in the `easy_variant` which has a throwing move will be wrapped in `recursive_wrapper` implicitly.

```c++
namespace safe_variant {
  template <typename ... Ts>
  using easy_variant = variant<wrap_if_thowing_move_t<Ts>...>;
}
```

where `wrap_if_throwing_move_t` is

```c++
namespace safe_variant {
  struct <typename T, typename = mpl::enable_if_t<std::is_nothrow_destructible<T>::value && !std::is_reference<T>::value>>
  struct wrap_if_throwing_move {
    using type = typename std::conditional<std::is_nothrow_move_constructible<T>::value,
                                           T,
                                           recursive_wrapper<T>>::type;
  };

  template <typename T>
  using wrap_if_throwing_move_t = typename wrap_if_throwing_move<T>::type;
}
```


Synopsis
========

The actual interface is in most ways the same as `boost::variant`, which strongly inspired this.  

(However, my interface is exception-free. If you want to have
analogues of the throwing functions in `boost::variant` you'll have to write them, which is pretty easy to do on top of the exception-free interface.)

```c++
namespace safe_variant {

  template <typename First, typename... Types>
  class variant {

    // Attempts to default construct the First type.
    // If First is not default-constructible then this is not available.
    variant();

    // Special member functions: Nothing special here
    variant(const variant &);
    variant(variant &&);
    ~variant() noexcept;

    // Only available if all input types are no-throw move constructible
    variant & operator=(variant &&);
    variant & operator=(const variant &);

    // Constructs the variant from a type outside the variant,
    // using iterative strategy described in docs.
    // (SFINAE expression omitted here)
    template <typename T>
    variant(T &&);

    // Constructs the variant from a "subvariant", that is, another variant
    // over a strictly smaller set of types, modulo recursive_wrapper.
    // (SFINAE expression omitted here)
    template <typename... OTypes>
    variant(const variant<Otypes...> &);

    template <typename... OTypes>
    variant(variant<Otypes...> &&);

    // Emplace ctor. Used to explicitly specify the type of the variant, and
    // invoke an arbitrary ctor of that type.
    template <typename T>
    struct emplace_tag {};

    template <typename T, typename... Args>
    explicit variant(emplace_tag<T>, Args && ... args);

    // Emplace operation
    // Force the variant to a particular value.
    // The user explicitly specifies the desired type as template parameter,
    // which must be one of the variant types, modulo const, recursive wrapper.
    // (There are actually two implementations of emplace, depending on whether
    // the invoked ctor is noexcept. If it is not, then a move is used for
    // strong exception-safety.)
    template <typename T, typename... Args>
    void emplace(Args &&... args);

    // Reports the runtime type. The returned value is an index into the list
    // <First, Types...>.
    int which() const;

    // Test for equality. The which values must match, and operator == for the
    // underlying values must return true.
    bool operator == (const variant &) const;
    bool operator != (const variant &) const;
  };

  // Apply a static_visitor to the variant. It is called using the current value
  // of the variant with its current type as the argument.
  // Any additional arguments to `apply_visitor` are forwarded to the visitor. 
  template <typename V, typename... Types, typename... Args>
  void apply_visitor(V && visitor, variant<Types...> && var, Args && ... args);

  // Access the stored value. Returns `nullptr` if `T` is not the currently
  // engaged type.
  template <typename T, typename ... Types>
  T * get(variant<Types...> * v);

  template <typename T, typename ... Types>
  const T * get(const variant<Types...> * v);

  // Returns a reference to the stored value. If it does not currently have the
  // indicated type, then the argument def is emplaced into the variant, and a
  // reference to that value, within the variant, is returned.
  // This is noexcept if T is no_throw_move_constructible.
  template <typename T, typename ... Types>
  T & get_or_default(variant<Types...> & v, T def = {});

  // Base class, provided to conveniently derive visitors from.
  // Analogous to boost::static_visitor
  template <typename T = void>
  class static_visitor {
  public:
    typedef T result_type;
  };

} // end namespace safe_variant
```


Compiler Compatibility
======================

`safe_variant` is targets the C++11 standard.

It is known to work with `gcc >= 4.9` and `clang >= 3.5`.  

(It used to work with `gcc-4.8`, but at some point that was lost, I'm not
 sure exactly why. `gcc-4.8` seems to have some `constexpr` troubles now.)

Usage
=====

This is a header-only C++11 template library. To use it, all you need to do is
add the `include` folder to your include path. Then use the following includes in your code.

Forward-facing includes:

- `#include <safe_variant/variant_fwd.hpp>`  
  Forward declares the `variant type`, `recursive_wrapper` type.  
- `#include <safe_variant/variant.hpp>`  
  Defines the variant type, as well as `apply_visitor`, `get`, `get_or_default` functions.  
- `#include <safe_variant/recursive_wrapper.hpp>`  
  Similar to `boost::recursive_wrapper`, but for this variant type.  
- `#include <safe_variant/static_visitor.hpp>`  
  Similar to `boost::static_visitor`, but for this variant type.
- `#include <safe_variant/variant_compare.hpp>`  
  Gets a template type `variant_comparator`, which is appropriate to use with `std::map` or `std::set`.  
  By default `safe_variant::variant` is not comparable.  
- `#include <safe_variant/variant_hash.hpp>`  
  Makes variant hashable. By default this is not brought in.
- `#include <safe_variant/variant_stream_ops.hpp>`  
  Gets ostream operations for the variant template type.  
  By default `safe_variant::variant` is not streamable.  
- `#include <safe_variant/variant_spirit.hpp>`  
  Defines customization points within `boost::spirit` so that `safe_variant::variant` can be used just like `boost::variant` in your `qi` grammars.

All the library definitions are made within the namespace `safe_variant`.

I guess I recommend that you use a namespace alias for that, e.g. `namespace util = safe_variant;`, or
use a series of using declarations. In another project that uses this library, I did this:


```c++
    // util/variant_fwd.hpp
    
    #include <safe_variant/variant_fwd.hpp>

    namespace util {
      using variant = safe_variant::variant;
    }
```

```c++
    // util/variant.hpp
    #include <safe_variant/variant.hpp>
    #include <safe_variant/static_visitor.hpp>
    #include <safe_variant/recursive_wrapper.hpp>
    #include <safe_variant/variant_hash.hpp>

    namespace util {
      using safe_variant::variant;
      using safe_variant::get;
      using safe_variant::get_or_default;
      using safe_variant::apply_visitor;
      using safe_variant::recursive_wrapper;
      using safe_variant::static_visitor;
    }
```

...

but you should be able to do it however you like of course.

Configuration
=============

There are three preprocessor defines it responds to:

- `SAFE_VARIANT_ASSUME_MOVE_NOTHROW`  
  Assume that moves of input types won't throw, regardless of their `noexcept`
  status. This might be useful if you are using old versions of the standard
  library and things like `std::string` are not no-throw move constructible for
  you, but you want `safe_variant::variant` to act as though they are. This will
  allow you to get assignment operators for the variant as though everything
  were move constructible, but if anything actually does throw you get UB.

- `SAFE_VARIANT_ASSUME_COPY_NOTHROW`
  Assumes that copies of input types won't throw, regardless of their `noexcept`
  status. This is pretty dangerous, it only makes sense in projects where you
  already assume that dynamic allocations will never fail and just want to go
  as fast as possible given that assumption. Probably you are using
  `-fno-exceptions` anyways and a custom allocator, which you monitor on the side
  for memory exhaustion, or something like this.

- `SAFE_VARIANT_DEBUG`  
  Turn on debugging assertions.

Licensing and Distribution
==========================

**safe variant** is available under the boost software license.

Performance Characteristics
===========================

Assignment
----------

The main point of variation among variant types is usually how they handle the
never-empty guarantee.  (Beyond that, they often vary significantly in how they
actually implement the storage, especially if they seek `constexpr` support.)

In `boost::variant`, the strategy is to make a heap-allocated backup copy of the
value, and then destroy the original and attempt to allocate the new value in the
in situ storage. If that operation throws, then the heap-allocated copy still
exists, and subsequent uses point to that. Any later type-changing assignments
will try to take place in situ, and the heap-allocated backup will remain until
one of those succeeds.

This provides the most comprehensive support, but it incurs an extra copy and
an extra dynamic allocation for each assignment. There's also some space overhead
for the pointer.

In the C++17 `std::variant`, the strategy is to relax the never-empty guarantee
to a "rarely empty" guarantee, by introducing an empty state which occurs when
construction fails. This greatly simplifies assignment and makes that more efficient,
but it may complicate visitation, depending on how concerned you are about the
empty state.

In `safe_variant`, the focus is on a less general case. We are mostly concerned
anyways with cases when you have many types in the variant which are implicitly
convertible, and for assignment we more or less assume that they are no-throw
move constructible. Via `recursive_wrapper`, we support all the other types as
well, with the proviso that they will be heap allocated rather than allocated
in situ. In comparison with `boost::variant`, this results in no extra calls
to copy constructors when we make an assignment, although it will result in
extra dynamic allocations. It also will impact speed of visitation, in the sense
that you must dereference an extra pointer to find the object -- boost::variant
tries to get the object in situ, and only puts it on the heap if an exception
is thrown. However, if exceptions are thrown regularly, then you would already
have had to tolerate this overhead with `boost::variant`. An advantage, though,
is that operations on `safe_variant` are relatively easy to reason about, as
there are no dynamic allocations taking place that you don't explicitly opt in
to. And besides, for the use cases where `safe_variant` is attractive in the
first place, the types will all likely be no-throw move constructible anyways.

Visitation
----------

The other major point of variation is the speed of visitation.

In `boost::variant`, at least the early implementations used `boost::mpl`, and
were limited to twenty or so types. `boost::variant` works even prior to the
variadic templates feature. Doing without variadic templates makes the header
pretty complicated and hurts the compile times, but its very helpful to speed of
visitation, which can be done using an explicit `switch` statement.

In a variadic template-based implementation, switch statements cannot be used,
because there is no pack-expansion analogue for switch statements. The most
common strategy that I saw is to declare an array of function pointers, and
fill it with pointers to a series of instantiations of template functions,
corresponding to indices in the list of input types. Then this array is indexed
and the appropriate function pointer is called, passing along the visitor,
the variant storage, etc. The array thus forms a little manual jump table of
sorts.

This implementation scales very well to large numbers of elements, but it has
the drawback that the function pointers cannot always be inlined by the compiler,
and so for relatively small numbers of elements, it can be outperformed by
another strategy. In the second strategy, a binary tree is formed which holds at
each leaf one of the input types. We then search the binary tree using the
"which" value which would have been the index to the jump table. When we arrive
at the leaf, we know the runtime-type of the value, and can invoke the visitor
appropriately. This implementation involves no function pointers, so the calls
can always be inlined -- but it may involve several branches. However, these
branches may benefit from branch prediction, and in practice, its quite common
to have variants with only a handful of types. Particularly when there is one
type which is the "most popular", branch prediction can significantly speed up
the visitation well beyond what you will see in benchmarks with random data,
which are already quite favorable to the "binary" search strategy for small
numbers of types.

Therefore, `safe_variant` uses a hybrid strategy. When the number of variants
is four or less, the binary search is used, and for more than that, the jump
table is used. 

There is a benchmarks suite in the `/bench` folder of the repository.

Compile-Times
-------------

I did not attempt to benchmark the compile-time performance, but I expect that
the differences would be negligible, especially in comparison to other common
libraries which are known to have heavy compile times.

Known issues
============

- There are still some issues with `noexcept` correctness which I would like to fix. It's mostly in order right now though.
- Currently, you cannot use `apply_visitor` with an rvalue-reference to the variant.  
  It must be an lvalue-reference or a const reference.  
  There is no reason for this restriction, but some of the dispatch code needs to be fixed
  to support this. I didn't need it in my original application.  
  It's okay for the visitor to be an rvalue-reference.  
- You can't use a lambda directly as a visitor. It needs to derive from `safe_variant::static_visitor`.
  This is similar to `boost::variant`. It could be fixed using `std::result_of`, that is a TODO item.
  Since generic lambdas are a C++14 feature anyways, this isn't that big a deal.
- No `constexpr` support. This is really extremely difficult to do in a variant at
  C++11 standard, it's only really feasible in C++14. If you want `constexpr` support
  then I suggest having a look at [`eggs::variant`](https://github.com/eggs-cpp/variant).
