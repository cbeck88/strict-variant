# strict variant

[![Build Status](https://travis-ci.org/cbeck88/strict-variant.svg?branch=master)](http://travis-ci.org/cbeck88/strict-variant)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/github/cbeck88/strict-variant?branch=master&svg=true)](https://ci.appveyor.com/project/cbeck88/strict-variant)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

Do you use `boost::variant` or one of the many open-source C++11 implementations of a "tagged union" or variant type
in your C++ projects?

`boost::variant` is a great library. I created `strict_variant` in order to address a few things about `boost::variant` that I didn't like.

The tl;dr version is that unlike `boost::variant` or `std::variant`, `strict_variant` will never throw an exception or make a dynamic allocation
in the effort of supporting types that have throwing moves. The default version will simply fail a static assert if this would happen. The
`strict_variant::easy_variant` will make allocations in this situation, so you can opt-in to that if you want, and these two versions of variant
"play nicely" together. This kind of thing is often a major concern in projects with realtime requirements, or in embedded devices, which may not
allow, or simply may not have these C++ features. If you are making a library that might be used in "conventional" projects that want the ease-of-use
that comes from `boost::variant`, but might also be used in projects with restrictive requirements, and you want to use a `variant` type as part of the
API, `strict_variant` might offer a way to keep everyone happy.

Besides this, there are some issues in the interface of variant that were addressed that make it more pleasant to use day-to-day IMHO.
(These were actually the original motivation of the project.)

- I didn't like that code like this may compile without any warning or error messages:

  ```c++
  boost::variant<std::string, int> v;  

  v = true;  
  ```
  
  I'd usually rather that my `variant` is more restrictive about what implicit conversions can happen.

- I *wanted* that things like this should compile and do what makes sense, even if overload resolution would
  be ambiguous.

  ```c++
  variant<bool, long, double, std::string> v;  

  v = true;  // selects bool
  v = 10;    // selects long 
  v = 20.5f; // selects double
  v = "foo"; // selects string
  ```

  I also wanted that such behavior (what gets selected in such cases) is portable.

  (For code examples like this, where `boost::variant` has unfortunate behavior, see "Abstract and Motivation" in the documentation.)

  In `strict_variant` we modify overload resolution in these situations by removing some candidates.

  For instance:
     * We eliminate many classes of problematic conversions, including narrowing and pointer conversions.
     * We prohibit standard conversions between `bool`, integral, floating point, pointer, character, and some other classes.  
     * We impose "rank-based priority". If two integer promotions are permitted
       but one of them is larger than another, the larger one gets discarded,  
       e.g. if `int -> long` and `int -> long long` are candidates,
       the `long long` is eliminated.
       
   See documentation for details.
  
- I didn't like that `boost::variant` will silently make backup copies of my objects. For instance, consider this simple program,
  in which `A` and `B` have been defined to log all ctor and dtor calls.
  
  ```c++
  int main() {
    using var_t = boost::variant<A, B>;
  
    var_t v{A()};
    std::cout << "1" << std::endl;
    v = B();
    std::cout << "2" << std::endl;
    v = A();
    std::cout << "3" << std::endl;
  }
  ```
  
  The `boost::variant` produces the following output:
  
  ```c++
  A()
  A(A&&)
  ~A()
  1
  B()
  B(B&&)
  A(const A &)
  ~A()
  B(const B &)
  ~A()
  ~B()
  ~B()
  2
  A()
  A(A&&)
  B(const B &)
  ~B()
  A(const A &)
  ~B()
  ~A()
  ~A()
  3
  ~A()
  ```
  
  This may be pretty surprising to some programmers.
   
  By contrast, if you use the C++17 `std::variant`, or one of the variants with
  "sometimes-empty" semantics, you get something like this (this output from `std::experimental::variant`)
  
  ```c++
  A()
  A(A&&)
  ~A()
  1
  B()
  ~A()
  B(B&&)
  ~B()
  2
  A()
  ~B()
  A(A&&)
  ~A()
  3
  ~A()
  ```
  
  This is much closer to what the naive programmer expects who doesn't know about internal
  details of `boost::variant` -- the only copies of his objects that exist are what he can see
  in his source code.
  
  This kind of thing usually doesn't matter, but sometimes if for instance you are
  debugging a nasty memory corruption problem (perhaps there's bad code in one of the objects contained
  in the variant), then these extra objects, moves, and copies, may make things incidentally more complicated.
  
  Here's what you get with `strict_variant`:
  
  ```c++
  A()
  A(A&&)
  ~A()
  1
  B()
  B(B&&)
  ~A()
  ~B()
  2
  A()
  A(A&&)
  ~B()
  ~A()
  3
  ~A()
  ```
  
  Yet, `strict_variant` does not have an empty state, and is fully exception-safe!

  (These examples from `gcc 5.4`, see code in [`example`](./example) folder.)

  To summarize the differences:

  - `std::variant` is rarely-empty, always stack-based. In fact, it's empty exactly
    when an exception is thrown. Later, it throws different exceptions if you try to visit
    when it is empty.
  - `boost::variant` is never-empty, usually stack-based. It has to make a dynamic allocation
    and a backup copy whenever an exception *could* be thrown, but that gets freed right after
    if an exception is not actually thrown.
  - `strict_variant` is never-empty, and stack-based exactly when the current value-type is
    nothrow moveable. It never makes a backup move or copy, and never throws an exception.
   
  Each approach has its merits. I chose the `strict_variant` approach because I find it
  simpler and it avoids what I consider to be drawbacks of `boost::variant` and `std::variant`. 
  And, if you manage
  to make all your types no-throw move constructible, which I often find I can, then `strict_variant`
  gives you optimal performance, the same as `std::variant`, without an empty state.


For an in-depth discussion of the design, check out the documentation.

For a gentle intro to variants, and an overview of strict-variant, see **slides**
from a talk I gave about this: \[[pptx](https://cbeck88.github.io/strict-variant/strict_variant.pptx)\]\[[pdf](https://cbeck88.github.io/strict-variant/strict_variant_static.pdf)\]


Documentation
=============

On [github pages](https://cbeck88.github.io/strict-variant/index.html).

Compiler Compatibility
======================

`strict_variant` targets the C++11 standard.

It is known to work with `gcc >= 4.8` and `clang >= 3.5`, and is tested against `MSVC 2015`.

`strict_variant` can be used as-is in projects which require `-fno-exceptions` and `-fno-rtti`.

Licensing and Distribution
==========================

**strict variant** is available under the boost software license.
