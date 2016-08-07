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
    variant v{variant::emplace_tag<my_type>, arg1, arg2, arg3};
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
the nothrow move-constructible members, which enjoy optimal performance like they
would in `std::variant`. We accomodates all members with throwing move by
putting them on the heap -- then the pointer can be moved into storage without
failure.

In comparison with `boost::variant`, this results in no extra calls
to copy constructors when we make an assignment.
It also will impact speed of visitation, in the sense
that you must dereference an extra pointer to find the object -- `boost::variant`
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
C++11. And also in that case, we enjoy the same performance as `std::variant`
without the added complexity of an empty state.

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
|             `boost::variant` |  6.481400 |  7.102700 |  7.851000 |  8.150300 |  8.705700 |  9.607100 |  9.010800 |  9.151600 |  9.319300 |  9.435100 |  9.369900 |       N/A |
|  libcxx (dev) `std::variant` |  8.157500 | 10.038500 | 11.234200 | 12.202000 | 12.081600 | 15.154700 | 15.588100 | 13.664700 | 13.287600 | 20.551800 | 19.929300 | 24.191200 |
|    `strict_variant::variant` |  0.538400 |  3.010500 |  5.066300 |  7.719000 |  9.092100 | 10.049600 | 11.931200 | 13.531000 | 14.647300 | 14.891000 | 16.392000 | 25.794200 |
|            `mapbox::variant` |  0.676200 |  2.958400 |  3.465800 |  4.423300 |  5.006200 |  6.020200 |  6.966400 |  7.431300 |  7.585400 |  8.796200 |  9.485000 | 13.482700 |
|             `juice::variant` |  8.331400 |  9.701500 | 10.550000 | 11.724400 | 12.618300 | 15.397400 | 18.612200 | 16.798500 | 19.801800 | 21.027400 | 21.903400 | 26.065400 |
|              `eggs::variant` |  6.805700 |  8.315000 |  8.948000 |  9.379700 | 10.019700 | 10.797000 | 10.483200 | 10.682700 | 10.854300 | 10.828700 | 10.955000 | 11.177400 |
| `std::experimental::variant` |  6.744500 |  8.357300 |  9.757800 | 10.487500 | 10.113300 | 10.634800 | 11.095100 | 11.595900 | 12.160900 | 19.551400 | 19.727200 | 22.055700 |


clang++ (3.8.0)
---------------

|              Number of types |         2 |         3 |         4 |         5 |         6 |         8 |        10 |        12 |        15 |        18 |        20 |        50 |
| ---------------------------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- | --------- |
|             `boost::variant` |  7.477200 |  6.645700 |  6.545900 |  6.628300 |  6.568700 |  6.564100 |  6.889100 |  6.581600 |  6.545600 |  7.651300 |  0.915300 |       N/A |
|  libcxx (dev) `std::variant` |  8.080700 | 10.121900 | 11.084000 | 12.564200 | 12.486700 | 13.248000 | 14.108300 | 14.250700 | 14.377300 | 14.378900 | 14.913100 | 14.945100 |
|    `strict_variant::variant` |  0.837900 |  1.889300 |  2.091400 |  4.484100 |  5.044800 |  5.205900 |  7.585900 |  8.154700 |  8.149400 | 10.217100 | 12.028300 | 16.273400 |
|            `mapbox::variant` |  0.865100 |  1.212400 |  2.521900 |  4.265600 |  3.997600 |  5.940000 |  6.685200 |  7.643000 |  8.871700 |  9.998200 | 10.028600 | 14.105200 |
|             `juice::variant` |  9.606700 | 10.345300 | 10.603600 | 10.897700 | 13.203500 | 14.954600 | 15.090100 | 17.874800 | 19.452000 | 19.677100 | 19.533400 | 25.147900 |
|              `eggs::variant` |  6.944900 |  8.718300 |  9.555800 | 10.104900 | 10.443600 | 11.002000 | 11.322000 | 11.809000 | 11.509700 | 11.656900 | 11.696600 | 12.044800 |
| `std::experimental::variant` |  7.009600 |  8.675900 |  9.564000 | 10.066800 | 10.524500 | 11.320100 | 11.098200 | 11.244600 | 11.586100 | 11.549700 | 11.686800 | 11.972000 |

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
- `strict_variant` from this repository at commit 94f80fe1cd0d34f8b49268e223f3dfca66605c3a

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

- Better docu regarding `recursive_wrapper` and design considerations
- Allocator support?
- No `constexpr` support. This is really extremely difficult to do in a variant at
  C++11 standard, it's only really feasible in C++14. If you want `constexpr` support
  then I suggest having a look at [`eggs::variant`](https://github.com/eggs-cpp/variant).
