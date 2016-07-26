# safe variant

[![Build Status](https://travis-ci.org/cbeck88/safe-variant.svg?branch=master)](http://travis-ci.org/cbeck88/safe-variant)
[![Coverage Status](https://coveralls.io/repos/cbeck88/safe-variant/badge.svg?branch=master&service=github)](https://coveralls.io/github/cbeck88/safe-variant?branch=master)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

Do you use `boost::variant` or one of the many open-source C++11 implementations of a "tagged union" or variant type
in your C++ projects?

Do you get annoyed that code like this will compile, without any warning or error message?

``
  boost::variant<std::string, int> v;  
  v = true;  
``

Do you get annoyed that code like this will compile on some machines, but not others?

``
  boost::variant<long, unsigned int> v;  
  v = 10;  
``


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

Overload resolution is a core C++ language feature, and in 90% of cases it works very well and does the right thing.

However, in the 10% of cases where overload resolution does the wrong thing, it can be quite difficult to work around it.
This includes the scenarios in which overload resolution is ambiguous, as well as the cases in which, due to some implicit conversion,
an overload is selected which the user did not intend.

Because integral types have so many permitted conversions, these problems are particularly obvious when you have a variant with
several integral types.

This happens commonly when using variant types to interface with some scripting language for instance. The typical dynamically-typed
scripting language will permit a variety of primitive values, so when binding to it, you may naturally end up with something like

```
    boost::variant<bool, int, float, std::string, ...>
```

**safe variant** therefore does not use overload resolution in its implementation.  

Instead, it uses a very simple iterative strategy.

- When the variant is constructed from a value, each type is checked one by one, to see if a *safe* conversion to that type is possible.
  If so, it is selected. If not, we check the next type. If no safe conversion is possible, then a compile-time error results.  
  This means that usually, you simply list your integral types in increasing order of rank, roughly, and it does the right thing.

- What conversions are safe?  
  I wrote a type trait that implements a strict notion of safety which was appropriate for the project in which
  I developed this. (See [1](include/safe_variant/conversion_rank.hpp), [2](include/safe_variant/safely_convertible.hpp)).
  - Conversions are not permitted between any two of the following classes:  
  Integral types, Floating point types, Character types, Boolean, Pointer types, and `wchar_t`.
  - If an integral or floating point conversion *could* be narrowing on some conforming implementation of C++, then it is not safe.  
  (So, `long` cannot be converted to `int`
  even if you are on a 32-bit machine and they have the same size for you, because it could be narrowing on a 64-bit machine.)
  - Signed can be promoted to unsigned, but the reverse is not allowed (since it is implementation-defined).
 
- You can force the variant to a particular type using the `emplace` template function. Rarely necessary but sometimes useful, and saves a `move`.

Another decision which I made, in order to side-step the "never empty" issue, is that any type used with the variant must be no-throw move constructible.
This allows the implementation to be very simple and efficient compared with some other variant types, which may have to make extra copies to facilitate
exception-safety, or make only a "rarely empty" guarantee.

This is enforced using static asserts, but sometimes that can be a pain if you are forced to use e.g. GCC 4-series versions of the C++ standard library which
are not C++11 conforming. So there is also a flag to turn the static asserts off, see `static constexpr bool assume_no_throw_move_constructible`.

The actual interface is in most ways the same as `boost::variant`, which strongly inspired this. (However, my interface is exception-free, if you want to have
analogues of the throwing functions in `boost::variant` you'll have to write them, which is pretty easy to do on top of the exception-free interface.)

So, keep in mind, this is not a drop-in replacement for `boost::variant` or one of the other versions, its semantics are fundamentally different.
But in scenarios like those it was designed for, it may be easier to reason about and less error-prone.

Compiler Compatibility
=============

`safe_variant` is coded to the C++11 standard.

This code has been tested with multiple versions of `gcc` and `clang` over an extended period of time.  
The oldest compilers that I tested with were `gcc >= 4.8` and `clang >= 3.5`.

Usage
=====

This is a header-only C++11 template library. To use it, all you need to do is
add the `include` folder to your include path. Then use the following includes in your code.

Forward-facing includes:

- `#include <safe_variant/variant_fwd.hpp>`  
  Forward declares the variant type, recursive_wrapper type.
- `#include <safe_variant/variant.hpp>`
  Defines the variant type, as well as `apply_visitor`, `get`, `get_or_default` functions.
- `#include <safe_variant/recursive_wrapper>`.
  Similar to `boost::recursive_wrapper`, but for this variant type.
- `#include <safe_variant/static_visitor.hpp>`
  Similar to `boost::static_visitor`, but for this variant type.
- `#include <safe_variant/variant_compare>`.
  Gets a template type `variant_comparator`, which is appropriate to use with `std::map` or `std::set`.
  By default `safe_variant::variant` is not comparable.
- `#include <safe_variant/variant_stream_ops>`.
  Gets ostream operations for the variant template type.
  By default `safe_variant::variant` is not streamable.
- `#include <safe_variant/variant_spirit.hpp>`
  Defines customization points within `boost::spirit` so that `safe_variant::variant` can be used just like `boost::variant` in your `qi` grammars.

All the library definitions are made within the namespace `safe_variant`.

I guess I recommend you to use a namespace alias for that, e.g. `namespace util = safe_variant;`, or
use a series of using declarations. In another project that uses this library, I did this:


```
    // util/variant_fwd.hpp
    
    #include <safe_variant/variant_fwd.hpp>

    namespace util {
      using variant = safe_variant::variant;
    }
```

```
    // util/variant.hpp
    #include <safe_variant/variant.hpp>
    #include <safe_variant/static_visitor.hpp>
    #include <safe_variant/recursive_wrapper.hpp>

    namespace util {
      using safe_variant::variant;
      using safe_variant::get;
      using safe_variant::apply_visitor;
      using safe_variant::get_or_default;
    }
```

...

but you should be able to do it however you like of course.


Licensing and Distribution
==========================

**safe variant** is available under the boost software license.
