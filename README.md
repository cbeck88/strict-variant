# strict variant

[![Build Status](https://travis-ci.org/cbeck88/strict-variant.svg?branch=master)](http://travis-ci.org/cbeck88/strict-variant)
[![Coverage Status](https://coveralls.io/repos/cbeck88/strict-variant/badge.svg?branch=master&service=github)](https://coveralls.io/github/cbeck88/strict-variant?branch=master)
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

On [https://cbeck88.github.io/strict-variant/index.html](github pages).

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

See documentation for how we handle the never-empty guarantee.


Compiler Compatibility
======================

`strict_variant` targets the C++11 standard.

It is known to work with `gcc >= 4.8` and `clang >= 3.5`.  

Licensing and Distribution
==========================

**strict variant** is available under the boost software license.

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
empty state. In essence we are sacrificing strong exception safety for only
basic exception safety, in a quest for performance.

In `strict_variant`, the focus is on a less general case. We basically "favor"
the nothrow move constructible members, which enjoy optimal performance like they
would in `std::variant`. We create a "slow track" which accomodates all members
with throwing moves, by simply always putting them on the heap.
In comparison with `boost::variant`, this results in no extra calls
to copy constructors when we make an assignment.
It also will impact speed of visitation, in the sense
that you must dereference an extra pointer to find the object -- boost::variant
always tries to get the object in situ, and only puts it on the heap if an exception
is thrown. However, if exceptions are thrown regularly, then you would already
have had to tolerate this overhead with `boost::variant`. An advantage, though,
is that operations on `strict_variant` are relatively easy to reason about, as
there are no dynamic allocations taking place that you don't explicitly opt in
to. And besides, for the use cases like those for which `strict_variant` was
originally designed and for which the "no unsafe conversions" property is
most relevant, the types will all likely be no-throw move constructible anyways.

Regardless, at least when your types are in fact no-throw move constructible,
we enjoy essentially the same interface as `boost::variant` updated to modern
C++, without the extra copies or dynamic allocations that were required prior to
C++11.

Visitation
----------

The other major point of variation among variant templates is the speed of visitation.

In `boost::variant`, at least the early implementations used `boost::mpl`, and
were limited to twenty or so types. `boost::variant` works even prior to the
variadic templates feature. Doing without variadic templates makes the header
pretty complicated and hurts the compile times, but its very helpful to speed of
visitation, which can be done using an explicit `switch` statement.

In a variadic template-based implementation, switch statements cannot be used,
because there is no pack-expansion analogue for switch statements. The most
common strategy that I saw is to declare an array of function pointers, and
fill it with a series of instantiations of a template function. Each pointer
corresponds to an one of the input types. Then this array is indexed
and the appropriate function pointer is called, passing along the visitor,
the variant storage, etc. The array thus forms a little manual jump table of
sorts.

This implementation is pretty straightforward, but it has the drawback that the
function pointers cannot always be inlined by the compiler,
and so for relatively small numbers of elements, it can be outperformed by
other strategies.

In a second strategy, a binary tree is formed which holds at
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
numbers of types. This is the strategy currently used by `strict_variant`.

A third strategy, naive tail recursion, is used by `mapbox::variant`.
Surprisingly (for me), this is the best performing strategy, for both compilers
and all regimes, according to my most recent benchmark investigations.

See below for benchmark data.

Compile-Times
-------------

I did not attempt to benchmark the compile-time performance, but I expect that
the differences would be negligible, especially in comparison to other common
libraries which are known to have heavy compile times.

Visitation Benchmarks
=====================

There is a [benchmarks suite](/bench) included in the repository.

*Take these benchmarks with a large grain of salt,* as actual performance will
depend greatly on success of branch prediction / whether the instructions in the
jump table are prefetched, and compiler inline decisions will be affected by
what the actual visitor is doing.

(From experience, these microbenchmarks are
extremely sensitive to the benchmark framework -- marking different components
at different steps as opaque to the optimizer has a very large effect. Feel free
to poke it and see what I'm talking about.)

But with that in mind, here are some
benchmark numbers for visiting ten thousand variants in randomly distributed
states, with varying numbers of types in the variant.

Benchmark numbers represent *average number of nanoseconds per visit*.

g++ (5.4.0)
-----------

|              Number of types |         2 |         3 |         4 |         5 |         6 |         8 |        10 |        12 |        15 |        18 |        20 |        50 |
| ---------------------------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- |
|             `boost::variant` |  6.401600 |  7.097300 |  8.054300 |  8.455100 |  8.512600 |  8.821300 |  9.025300 |  9.184600 |  9.282000 |  9.429400 |  9.428000 |       N/A |
|              `eggs::variant` |  6.745800 |  8.519000 |  8.932000 |  9.355500 |  9.878400 | 10.131900 | 10.502100 | 10.648500 | 10.766800 | 10.810700 | 11.082700 | 11.135300 |
|  libcxx (dev) `std::variant` |  8.124100 | 10.115100 | 11.158700 | 12.137400 | 12.205300 | 15.015400 | 14.729900 | 13.449000 | 13.284800 | 20.426200 | 20.371100 | 23.552300 |
|            `mapbox::variant` |  0.704700 |  2.693700 |  3.327500 |  4.363400 |  5.023000 |  6.056900 |  6.875800 |  7.403600 |  7.535600 |  8.684800 |  9.435200 | 13.606900 |
|             `juice::variant` |  8.359100 |  9.732000 | 10.542600 | 11.916000 | 13.019800 | 15.827900 | 18.159000 | 16.783400 | 19.671300 | 21.399900 | 22.595600 | 25.679100 |
|      `strict_variant::variant` |  0.528500 |  2.997200 |  4.975100 |  7.948200 |  8.812900 |  9.894800 | 11.830600 | 13.579700 | 14.692000 | 14.726800 | 16.803200 | 24.791100 |
| `std::experimental::variant` |  6.851700 |  8.374500 |  9.230800 |  9.691900 | 10.173600 | 10.589700 | 11.145200 | 11.605700 | 12.364800 | 19.994500 | 18.656700 | 21.930500 |

clang++ (3.8.0)
---------------

|              Number of types |         2 |         3 |         4 |         5 |         6 |         8 |        10 |        12 |        15 |        18 |        20 |        50 |
| ---------------------------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- |
|             `boost::variant` |  7.615800 |  6.611500 |  6.587400 |  6.598600 |  6.531100 |  6.554200 |  6.573100 |  6.542300 |  6.572600 |  7.663900 |  0.978600 |       N/A |
|              `eggs::variant` |  6.925900 |  8.672100 |  9.811700 | 10.357700 | 10.474400 | 10.900800 | 11.219100 | 11.358900 | 11.914600 | 12.034900 | 11.703300 | 12.041200 |
|  libcxx (dev) `std::variant` |  8.372000 | 11.307300 | 11.786800 | 12.869200 | 13.686700 | 14.720300 | 14.240400 | 14.233100 | 14.372100 | 14.371900 | 14.434000 | 15.388300 |
|            `mapbox::variant` |  0.828200 |  1.195100 |  2.513900 |  4.224100 |  3.994300 |  6.023600 |  6.724500 |  7.614300 |  8.931000 |  9.974600 | 10.066500 | 13.406500 |
|             `juice::variant` |  8.348200 |  9.730200 | 10.587900 | 10.857300 | 11.985100 | 15.497900 | 15.142700 | 17.893400 | 19.209700 | 20.189300 | 20.337800 | 27.006100 |
|      `strict_variant::variant` |  0.799700 |  1.872300 |  2.078000 |  4.340500 |  5.089000 |  5.516400 |  7.603800 |  7.984200 |  8.152600 | 10.277700 | 12.062700 | 15.266700 |
| `std::experimental::variant` |  7.142900 |  8.687600 |  9.527800 | 10.099500 | 10.451000 | 10.822700 | 11.185500 | 11.464700 | 11.515100 | 11.603700 | 11.668800 | 11.983700 |


configuration data
------------------

The settings used for these numbers are:
```
  seq_length = 10000
  repeat_num = 1000
```

Test subjects:

- `boost::variant` at version 1.58
- `eggs::variant` from [this repository](https://github.com/eggs-cpp/variant) at commit 1692cb849311cd8dfe77146ad21b4f00299a68cd
- `juice::variant` from [this repository](https://github.com/jarro2783/thenewcpp) at commit 5572c1b9017f50e9b39d9bf33b1a6719e1983476
- `mapbox::variant` from [this repository](https://github.com/mapbox/variant) at commit 388376ac9f0102feba2d2122873b08e15a66a879
- `std::experimental::variant` from [this repository](https://github.com/mpark/variant)
- libcxx `std::variant` draft at `variant` branch from [this repository](https://github.com/efcs/libcxx/tree/variant) at commit d93edff042e7b9d333eb5b6f16145953e00fd182
- `strict_variant` from this repository at commit 5e17d9edabe6a35ed2af8bc9c5d0dde3bd42efe4

My `/proc/cpuinfo` looks like this:

```
$ cat /proc/cpuinfo 
processor	: 0
vendor_id	: GenuineIntel
cpu family	: 6
model		: 78
model name	: Intel(R) Core(TM) i7-6600U CPU @ 2.60GHz
stepping	: 3
microcode	: 0x6a
cpu MHz		: 438.484
cache size	: 4096 KB
physical id	: 0
siblings	: 4
core id		: 0
cpu cores	: 2
apicid		: 0
initial apicid	: 0
fpu		: yes
fpu_exception	: yes
cpuid level	: 22
wp		: yes
flags		: fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch epb intel_pt tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm mpx rdseed adx smap clflushopt xsaveopt xsavec xgetbv1 dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp
bugs		:
bogomips	: 5615.78
clflush size	: 64
cache_alignment	: 64
address sizes	: 39 bits physical, 48 bits virtual
power management:
```

with three additional cores identical to that one.

Known issues
============

- Need to fixup operator declarations / inclusions
- Fixup noexcept in recursive_wrapper
- Review move of recursive_wrapper vs. variant
- No `constexpr` support. This is really extremely difficult to do in a variant at
  C++11 standard, it's only really feasible in C++14. If you want `constexpr` support
  then I suggest having a look at [`eggs::variant`](https://github.com/eggs-cpp/variant).
