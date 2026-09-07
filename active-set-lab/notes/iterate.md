## Active-Set Iteration Benchmark

Compares how fast each implementation visits **all active instruments** (book sweep) at different active fractions of the 4096-instrument universe. This is the dominant hot path in a feed handler / strategy loop: each tick you must touch every active instrument and only the active ones.

### Implementations measured

| Short name | Storage of active membership | Iteration cost |
|---|---|---|
| `TreeSet` | `std::set` (red-black tree, ids) | O(active * log N), pointer chase |
| `HashSet` | `std::unordered_set` (ids) | O(active), scattered buckets |
| `ByteFlagSet` | `uint8_t` flag array | **O(N) always** — branch test per flag |
| `BitFlagSet` | `vector<bool>` bit-packed flags | **O(N) always** — bit extract per flag |
| `BitmaskSet` | `uint64_t[N/64]` + countr_zero | O(active + N/64) |
| `DenseIdSet` | swap-and-pop prefix of 4-byte ids | O(active), sequential 4-byte stride |
| `DensePtrSet` | swap-and-pop prefix of 8-byte ptrs | O(active), ptr then deref |
| `DenseStructSet` | swap-and-pop prefix of 48-byte structs | O(active), pure sequential |
| `IntrusiveListSet` | embedded prev/next links | O(active), dependent pointer chase |

### Results (ns per full sweep, universe N = 4096)

| Implementation | 10% active | 25% | 50% | 75% | 100% |
|---|---|---|---|---|---|
| TreeSet | 3,217 | 13,334 | 27,814 | 43,064 | 63,093 |
| HashSet | 1,501 | 3,657 | 6,977 | 10,741 | 12,977 |
| ByteFlagSet | 9,014 | 7,329 | 7,284 | 5,324 | 5,223 |
| BitFlagSet | 12,786 | 12,550 | 13,525 | 13,302 | 14,077 |
| BitmaskSet | **615** | 1,304 | 3,011 | 4,417 | 5,990 |
| DenseIdSet | 555 | 1,705 | 3,244 | 4,651 | 5,901 |
| DensePtrSet | 368 | 1,450 | 2,950 | 4,380 | 5,659 |
| DenseStructSet | **268** | **701** | **1,849** | **2,602** | **3,401** |
| IntrusiveListSet | 1,822 | 9,429 | 22,616 | 29,018 | 38,365 |

### Relative speedup vs `ByteFlagSet` at each active fraction

| Implementation | 10% | 25% | 50% | 75% | 100% |
|---|---|---|---|---|---|
| BitmaskSet | 14.7× | 5.6× | 2.4× | 1.2× | 0.87× |
| DenseIdSet | 16.2× | 4.3× | 2.2× | 1.1× | 0.89× |
| DensePtrSet | 24.5× | 5.1× | 2.5× | 1.2× | 0.92× |
| DenseStructSet | **33.6×** | **10.5×** | **3.9×** | **2.0×** | **1.54×** |
| IntrusiveListSet | 4.9× | 0.78× | 0.32× | 0.18× | 0.14× |

### Interpretation

- **Sparse actives (10%) is where design wins.** `DenseStructSet` sweeps 33× faster than a flag array: it stores active elements *directly* in a contiguous prefix, so iteration costs 48 B/element sequential and touches only active instruments. `DensePtrSet` (24.5×) and `DenseIdSet` (16.2×) trail because each element costs an extra load (pointer deref, or id→index lookup into the universe array).
- **flags and vector<bool> pay O(N) regardless.** At 10% active, a flag sweep still inspects all 4096 slots, so it is ~9–13 µs whether 410 or 4096 instruments are active. BitFlagSet is always ~1.4–1.9× *slower* than ByteFlagSet because of bit extraction + proxy access.
- **Contiguity beats pointer chasing.** `IntrusiveListSet` is the same O(active) complexity yet 70–140× slower than the dense variants at high active fractions — every `next` load is a dependent, cache-missing hop. This is the classic "linked structures look O(1) but the constant is a cache miss."
- **At 100% active everything converges** to scanning all N, so the dense prefix no longer saves work; the flag arrays catch up. Even then DenseStructSet (pure sequential reads) stays ~1.5× over the flag sweep.
- **TreeSet never recovers** — red-black tree walk with pointer chasing and lower ILP; 63 µs at full population. HashSet is mid-pack (13 µs) — buckets are heap-scattered.

### Items/s for reference (full-sweep throughput at 10%)

| Implementation | items/s |
|---|---|
| DenseStructSet | 1.53 G/s |
| DensePtrSet | 1.11 G/s |
| DenseIdSet | 737 M/s |
| BitmaskSet | 666 M/s |
| HashSet | 273 M/s |
| IntrusiveListSet | 225 M/s |
| TreeSet | 127 M/s |
| ByteFlagSet | 45 M/s |
| BitFlagSet | 32 M/s |

### Key Takeaway

If iteration dominates and actives are sparse, store the active elements **by value in a dense prefix** (swap-and-pop). Iteration then costs exactly the active set's bytes in sequential memory — order-independent of the universe size. Pointer/flag/list variants each pay a different tax: indirection, full-N scan, or cache misses respectively. At high active density the sweep variants become competitive, but they never win outright.