# Adversary workload contract

Maintainer: Mayank Moyal.

All generators are C++17 and independent of the two structures. Keys use signed 64-bit `long long`, checked at compile time. Generated domains are [0,16n), with n in [1,1000000]. External replay tests also exercise LLONG_MIN and LLONG_MAX. Population and domain validation reject invalid n.

The generator and trace-validator CLI arguments `n`, `pairs` and `seed` must contain decimal digits only. Signs, whitespace, fractional values, trailing characters and uint64 overflow are rejected before trace output. Leading zeroes are allowed. Population is restricted to [1,1000000]; pair counts and seeds accept [0,18446744073709551615]. A zero-pair trace contains setup only. This rule does not change the separate measurement driver's `trial` and floating-point `alpha` arguments.

`core.h` specifies SplitMix64, rejection sampling for unbiased bounded integers and Fisher-Yates shuffling. Its arithmetic is explicitly unsigned. It does not depend on implementation-specific std::shuffle or distribution mappings. Seeds, algorithm version and parameters reproduce the same initial order and operation stream across C++17 platforms with 64-bit unsigned integers.

The initial population is the first n values of a shuffled permutation of the 16n-key domain, preserving that insertion order. It uses a separate initialization stream. Workload and randomized deletion-policy streams are separate; generators never read the measured structures' RNGs.

Text format:

```text
# churn-trace-v1 kind=churn n=3 pairs=1 seed=42 mode=exact
I 10
I 2
I 7
D 2
I 2
```

The numbers above illustrate syntax, not the actual seed-42 output. Skip comment lines. A churn trace begins with n successful initial insertions, then exactly `pairs` D/I pairs. A deletion must name a present key; the following insertion must name a key absent immediately after deletion. Population is n at every pair boundary. The just-deleted key is allowed as insertion; exact-fit workloads always use it. An insertion-only trace has kind=insertions, contains n I operations, and has no fixed-size churn pairs. Foundation output has kind=initial and contains setup only.

FNV-1a-64 trace hash starts at 14695981039346656037. For every operation, hash its ASCII I or D byte, then the key converted modulo 2^64 as eight little-endian bytes. Multiply by 1099511628211 after each XOR, wrapping modulo 2^64. Include setup operations in the cumulative hash. This is a reproducibility checksum, not a security hash; raw CSV files use SHA-256 in their manifests.

To compare strategies, replay the same initial insertion order and operation trace. Generate adaptive attacks once against the documented shape model and replay them unchanged. Never regenerate them independently against each measured strategy's shape.

Standalone foundation:

```powershell
g++ -std=c++17 -O2 adversary/foundation.cpp -o foundation.exe
.\foundation.exe 1000 42
```

The generator headers and CLIs follow this contract. See adversary/README.md for usage. Do not infer that a tested candidate is globally optimal.
