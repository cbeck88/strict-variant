Benchmarks:
===========

This is a benchmark framework that measures speed of static visitation for three different variant types.

- `safe_variant::variant`
- `std::experimental::variant` (as [provided by mpark](https://github.com/mpark/variant))
- `boost::variant` (using whatever boost version is installed at environment variable BOOST_ROOT or /usr/include)

For each tested variant, it generates an executable which tests the cases of 2, 3, 4, 5, 6, 8, 15, 20, and 50 items in a variant.
It tests it on a random sequence of variants formed using `std::mt19937` with seed `42`, of length 10,000,000. (See [Jamroot.jam](/bench/Jamroot.jam)).

You must build using `b2`.

Test executables are produced in `/bench/stage`.

Use `./run.sh` to do a clean build and run in one operation.
