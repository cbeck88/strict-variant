# strict variant

[![Build Status](https://travis-ci.org/cbeck88/strict-variant.svg?branch=master)](http://travis-ci.org/cbeck88/strict-variant)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/github/cbeck88/strict-variant?branch=master&svg=true)](https://ci.appveyor.com/project/cbeck88/strict-variant)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

Do you use `boost::variant` or one of the many open-source C++11 implementations of a "tagged union" or variant type
in your C++ projects?

Do you get annoyed that code like this may compile, without any warning or error message?

```c++
  boost::variant<std::string, int> v;  

  v = true;  
```

Do you get annoyed that code like this may compile on some machines, but not others?

```c++
  boost::variant<long, unsigned int> v;  

  v = 10;  
```

Do you crave the performance benefits of `std::variant`, but don't want to have
to put up with an empty state?

If so, then this may be the variant type for you.

**strict variant** is yet another C++11 variant type, with the twist that it prevents "unsafe" implicit conversions
such as narrowing conversions, conversions from bool to other integral types, pointer conversions, etc., and handles
overload ambiguity differently from other C++ variant types.

It may be well-suited for use in scenarios where you need to have a variant holding multiple different integral types,
and really don't want to have any loss of precision or any "gotcha" conversions happening.

**strict variant** provides a strict *never-empty guarantee* like `boost::variant`, for ease of visitation. And it is *strongly exception-safe*, having
rollback semantics if an assignment throws an exception. But it does not incur the cost of
backup copies which `boost::variant` pays, due to a different implementation approach. In some cases this allows
improved performance.

Documentation
=============

On [github pages](https://cbeck88.github.io/strict-variant/index.html).

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

One case when this might cause problems for you is when using variant types to interface with some scripting language for instance. The typical dynamically-typed
scripting language will permit a variety of primitive values, so when binding to it, you may naturally end up with something like

```c++
  boost::variant<bool, int, float, std::string, ...>
```

**strict variant** therefore uses SFINAE to modify the overload resolution process.

- When the variant is constructed from a value, each type is checked to see if a *safe* conversion to that type is possible.
  If not, then it is eliminated from overload resolution.

- To prevent ambiguity of assignments with e.g. integral types, any integral types will be added in stages increasing order
  of rank -- when a safe conversion becomes available, overload resolution is performed then without considering any larger
  matches. For example, if you assign an `int` to a variant containing `long` and `long long`, it will select `long` rather
  than reporting an ambiguous overload. This is true more generally for character types and floating point types.

- What conversions are "safe"?  
  I wrote a type trait that implements a strict notion of safety which was appropriate for the project in which
  I developed this. (See [1](include/strict_variant/conversion_rank.hpp), [2](include/strict_variant/safely_constructible.hpp)).
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
    variant v{emplace_tag<my_type>, arg1, arg2, arg3};
  ```

So, keep in mind, this is not a drop-in replacement for `boost::variant` or one of the other versions, its semantics are fundamentally different.
But in scenarios like those it was designed for, it may be easier to reason about and less error-prone.

Never-Empty Guarantee
=====================

See documentation for how we handle this.

Compiler Compatibility
======================

`strict_variant` targets the C++11 standard.

It is known to work with `gcc >= 4.8` and `clang >= 3.5`, and is tested against `MSVC 2015`.

Licensing and Distribution
==========================

**strict variant** is available under the boost software license.
