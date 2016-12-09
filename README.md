# strict variant

[![Build Status](https://travis-ci.org/cbeck88/strict-variant.svg?branch=master)](http://travis-ci.org/cbeck88/strict-variant)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/github/cbeck88/strict-variant?branch=master&svg=true)](https://ci.appveyor.com/project/cbeck88/strict-variant)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

Do you use `boost::variant` or one of the many open-source C++11 implementations of a "tagged union" or variant type
in your C++ projects?

I created `strict_variant` in order to address a few things about `boost::variant` that I didn't like.

- I didn't like that code like this may compile without any warning or error messages:

  ```c++
  boost::variant<std::string, int> v;  

  v = true;  
  ```
  
  I'd usually rather that my `variant` is more restrictive about what implicit conversions can happen.

- I didn't like that different integral conversions might be selected on different machines:

  ```c++
  boost::variant<long, unsigned int> v;  

  v = 10;
  ```
  
  The value of `v.which()` here can depend on implementation-defined details, which was bad for my application.
  
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
  
  `boost::variant` produces the following output:

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
  "sometimes-empty" semantics, you get somehting like this (this output from `std::experimental::variant`)
  
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
  in the variant),
  these extra objects, moves, and copies, may make things incidentally more complicated.
  
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

  (See code in [`example`](./example) folder.)

  To summarize the differences:

  - `std::variant` is rarely-empty, always stack-based. If an exception is thrown you end up
    in the empty state, and get exceptions later if you try to visit.
  - `boost::variant` is never-empty, usually stack-based. It has to make a dynamic allocation
    and a backup copy whenever an exception *could* be thrown, but that gets freed right after
    if an exception is not actually thrown.
  - `strict_variant` is never-empty, and stack-based exactly when the current value-type is
    nothrow moveable. It never makes a backup move or copy, and never throws an exception.
   
  Each approach has its merits. I chose the `strict_variant` approach because I find it
  simpler and it avoids the drawbacks of `boost::variant` and `std::variant`. If you manage
  to make all your types no-throw move constructible, which often I find I can, then `strict_variant`
  gives you optimal performance, the same as `std::variant`, without an empty state.


For an in-depth discussion of the design, check out the documentation.

Documentation
=============

On [github pages](https://cbeck88.github.io/strict-variant/index.html).

Compiler Compatibility
======================

`strict_variant` targets the C++11 standard.

It is known to work with `gcc >= 4.8` and `clang >= 3.5`, and is tested against `MSVC 2015`.

Licensing and Distribution
==========================

**strict variant** is available under the boost software license.
