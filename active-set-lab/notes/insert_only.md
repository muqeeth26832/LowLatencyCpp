## Active-Set Insert-Only Benchmark

Cold-build workload: start from an empty active set and insert all 4096 instruments, in a deterministic shuffled order, per iteration. Measures the raw cost of growing each structure.

### Results (ns for 4096 inserts)

| Implementation | Total (ns) | ns/insert | inserts/s |
|---|---|---|---|
| ByteFlagSet | **4,262** | 1.04 | 961 M |
| IntrusiveListSet | 5,107 | 1.25 | 802 M |
| DensePtrSet | 5,916 | 1.44 | 696 M |
| BitmaskSet | 7,163 | 1.75 | 572 M |
| BitFlagSet | 7,613 | 1.86 | 538 M |
| DenseIdSet | 10,144 | 2.48 | 404 M |
| DenseStructSet | 29,519 | 7.21 | 139 M |
| HashSet | 325,925 | 79.6 | 12.7 M |
| TreeSet | 811,402 | 198.1 | 5.05 M |

### Interpretation

- **ByteFlagSet is the cheapest insertion**: one byte write per id, fully sequential, zero accounting. At 4096-universe scale it is ~1 ns/insert of pure memory-bandwidth.
- **swap-and-pop storing ids needs bookkeeping**: DenseIdSet pays a slot-array write + a back-pointer update on every insert, landing at 2.5 ns. DensePtrSet is faster (1.44 ns) because the slot back-pointer lives inside the payload (one fewer array), but it *mutates* shared universe memory.
- **DenseStructSet is the slowest dense variant (7.2 ns)**: a swap-and-pop by value must copy the whole 48-byte struct on every insert. Insert-only is exactly where "store structs" hurts.
- **STL containers are two orders of magnitude slower** on bulk build: HashSet 80 ns/insert (hashing + bucket alloc), TreeSet 198 ns/insert (log-N rotations + node alloc). Here even a bitmask beats `std::unordered_set` by 45×.

### Key Takeaway

Insert-only favors the simplest possible write: a flag byte or a bit. Dense structures pay for their online bookkeeping, and struct-by-value pay for memcpy. But a 2.5 ns insert is only meaningful if you also need fast iteration — few systems build a set then never read it.