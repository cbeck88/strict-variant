Benchmarks:
===========

This is a benchmark framework that measures speed of static visitation for three different variant types.

- `safe_variant::variant`
- `std::experimental::variant` (as [provided by mpark](https://github.com/mpark/variant))
- `boost::variant` (using whatever boost version is installed at environment variable BOOST_ROOT or /usr/include)

For each tested variant, it generates an executable which tests the cases of 2, 3, 4, 5, 6, 8, 10, 15, 20, and 50 items in a variant.
It tests it on a random sequence of variants of a given length, currently 10000, and this is repeated 1000 times.
(See [Jamroot.jam](/bench/Jamroot.jam)).

You must build using `b2`.

Test executables are produced in `/bench/stage`.

Use `./run.sh` to do a clean build and run all benchmarks.

There is also a `./generate_asm.sh` script which will generate assembly for each of the variant types, at some particular configuration.

For additional comments and benchmark work on what is being tested here, check out an earlier stackoverflow question:
http://stackoverflow.com/questions/32235855/switch-statement-variadic-template-expansion/32235928#32235928
