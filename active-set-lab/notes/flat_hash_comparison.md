## Flat Hash Set (flat_hash_set) vs the active-set designs

A 10th implementation, added to test the "industry lookup answer": a
flat open-addressing hash set storing ids inline in one contiguous
power-of-two table with linear probing (the poor-man's `absl::flat_hash_set`).
Unlike `std::unordered_set`'s scattered linked buckets, all keys live in one
array — so lookups stay cache-friendly.

### Grouping by family

| Family | Members | Memory | Iteration |
|---|---|---|---|
| Node containers | TreeSet, HashSet, FlatHashSet, IntrusiveList | scattered / inline | O(capacity) walk |
| Sweep | ByteFlag, BitFlag | N bytes / N bits | O(N), tests everything |
| Dense prefix | DenseId, DensePtr, DenseStruct, Bitmask | O(active) | **O(active)** |

### FlatHashSet results (ns)

| Workload | FlatHashSet | vs DenseStruct | vs HashSet |
|---|---|---|---|
| Iterate 10% | 1,895 | 7.3× slower | 1.3× slower than HashSet |
| Iterate 100% | 16,142 | ~5× slower | 1.2× slower |
| Insert-only | 72,270 | 2.4× slower | 4.5× FASTER |
| Remove-only | 92,204 | 1.7× slower | 3.1× FASTER |
| Churn 10% | 10,083 | 1.1× slower | 1.9× FASTER |
| Churn 90% | 93,065 | 1.1× slower | 1.9× FASTER |
| Realistic 10% | 66,724 | 10× slower | 2.3× slower |
| Realistic 90% | 107,290 | 2.3× slower | ~equal |

### Interpretation

- **FlatHashSet does beat `std::unordered_set` on mutation** (insert 4.5×,
  remove 3.1×, churn ~1.9× faster): that is the whole Swiss-table win –
  contiguous probing beats chasing linked buckets. The case the industry
  actually cares about for hash sets.
- **But it still loses badly on iteration**, which is *our* hot path: a hash
  set iterates over **capacity (2N slots)**, not over active elements, and
  must skip tombstones/empties. At 10% active it is 7× slower than
  by-value swap-and-pop, and in the realistic tick 10× slower.
- **It is not faster than HashSet on iteration** (1.2–1.3× *slower*) — the
  contiguous table and the linked buckets both cost O(capacity) per sweep.
  Swiss tables (absl) close some of this with SIMD word-scan, but the bound
  is still capacity, not active.

### Conclusion

`flat_hash_set` is the right tool when **lookup-rate ≳ iteration-rate** and
you want cache-friendly membership checks — it handily beats
`std::unordered_set` there. It is the wrong tool for an
**iteration-dominant, sparse-active** sweep, which is exactly our use case:
the dense prefix designs stay O(active) and ~7× faster. This is a clean
demonstration that "a better hash set" does not fix "iteration is the hot
path" — only a container whose iteration is proportional to the live set
does.

### Where FlatHashSet ranks (mutation vs iteration trade)

- Best mutator of the lookup-family (beats `HashSet`)
- Worst sweeper of the flat structures (only TreeSet-style in-between)
- Recommended only if you also do frequent `contains()` and can accept
  O(capacity) iteration
