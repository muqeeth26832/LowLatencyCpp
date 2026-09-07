## Active-Set Remove-Only Benchmark

Teardown workload: start from a fully-populated 4096-instrument active set and erase every instrument, in deterministic shuffled order, per iteration. Measures the cost of shrinking each structure.

### Results (ns for 4096 erases)

| Implementation | Total (ns) | ns/erase | erases/s |
|---|---|---|---|
| ByteFlagSet | **9,283** | 2.27 | 441 M |
| BitmaskSet | 13,745 | 3.36 | 298 M |
| BitFlagSet | 16,916 | 4.13 | 242 M |
| DenseIdSet | 18,669 | 4.56 | 220 M |
| IntrusiveListSet | 27,107 | 6.62 | 151 M |
| DensePtrSet | 39,114 | 9.55 | 105 M |
| DenseStructSet | 53,520 | 13.07 | 77 M |
| HashSet | 287,038 | 70.1 | 14.3 M |
| TreeSet | 1,569,490 | 383.2 | 2.62 M |

### Interpretation

- **Removal is the "swap" tax.** Erasing from a random position forces the swap-and-pop moves: one element must be moved to keep the prefix dense, its slot back-pointer updated, and the hole zeroed. `DenseIdSet` moves a 4-byte id (4.6 ns); `DensePtrSet` moves an 8-byte pointer (9.6 ns — plus it must patch the moved payload's slot); `DenseStructSet` moves a whole 48-byte struct (13.1 ns).
- **Flags and bitmasks erase in-place** — one byte/word clear, no element movement — so they remain the cheapest (2.3 ns / 3.4 ns).
- **IntrusiveListSet** should be O(1) per unlink, but at 6.6 ns it is beat by the flat arrays that don't reorder anything. Its unlink touches 3 neighbouring nodes' cache lines.
- **STL containers again collapse**: TreeSet needs 383 ns per erase (log-N rebalance + dealloc); `std::unordered_set` 70 ns.

### Key Takeaway

Remove-heavy workloads reward structures that clear state in place. Dense swap-and-pop pays its O(1) guarantee through element movement — cheap when elements are small (id), expensive when they are structs. This is the mirror image of the iterate benchmark: iteration loves dense by-value, removal hates it.