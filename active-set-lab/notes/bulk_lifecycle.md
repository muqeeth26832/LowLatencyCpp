## Active-Set Bulk Lifecycle Benchmark

Churn-in-bulk: per iteration insert all 4096 instruments then erase all 4096 (2N ops). Models an instrument universe that fully re-activates then fully deactivates between sessions/ticks at coarse granularity.

### Results (ns for 8192 ops = 4096 insert + 4096 erase)

| Implementation | Total (ns) | ns/op | ops/s |
|---|---|---|---|
| ByteFlagSet | **9,510** | 1.16 | 863 M |
| BitmaskSet | 13,810 | 1.69 | 594 M |
| BitFlagSet | 16,989 | 2.07 | 484 M |
| DenseIdSet | 18,919 | 2.31 | 433 M |
| IntrusiveListSet | 25,071 | 3.06 | 327 M |
| DensePtrSet | 41,707 | 5.09 | 197 M |
| DenseStructSet | 53,540 | 6.54 | 153 M |
| HashSet | 283,864 | 34.6 | 28.9 M |
| TreeSet | 1,590,981 | 194.2 | 5.16 M |

### Interpretation

- The lifecycle result is essentially **insert + erase summed**, so rankings match the two constituent benchmarks. ByteFlagSet (clear/set a byte) and BitmaskSet (OR/ANDN a word) lead.
- **DenseIdSet holds its own at 2.3 ns/op**: small 4-byte element means both the insert bookkeeping and the erase swap are cheap.
- The gap widens at the dense-heavy end: DenseStructSet is 5.6× slower than ByteFlagSet because every op copies a 48-byte struct. DensePtrSet (5.1 ns) sits between id and struct.

### Key Takeaway

Coarse bulk churn is the domain of flags and bitmasks: neither of them reorders elements, so insert and erase are pure state writes. Any structure that must keep a dense prefix pays movement on both sides of the lifecycle.