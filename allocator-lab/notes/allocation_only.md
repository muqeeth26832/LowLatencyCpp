## Benchmark Results

Workload: **100,000 allocations of a 32-byte `Order` object**

| Allocator   | Time (ns) | Approx. ns/allocation |
| ----------- | --------: | --------------------: |
| `std::new`  | 8,527,404 |                ~85 ns |
| Arena       | 1,642,903 |                ~16 ns |
| Memory Pool | 4,052,560 |                ~40 ns |
| Slab        | 4,736,032 |                ~47 ns |

### Relative Speedup vs `std::new`

| Allocator   |  Speedup |
| ----------- | -------: |
| Arena       | **5.2×** |
| Memory Pool | **2.1×** |
| Slab        | **1.8×** |

### Interpretation

* **Arena** is the fastest because allocation is essentially just advancing a pointer within a preallocated buffer.
* **Memory Pool** avoids heap allocations by reusing fixed-size blocks from a free list, reducing allocation overhead.  * **Slab** builds on memory pools and adds size-class selection, introducing a small amount of extra work per allocation.
* **`std::new`** is the most flexible allocator but pays additional costs for heap management and bookkeeping.

These results show the typical trade-off between **flexibility** and **allocation latency**: specialized allocators achieve lower allocation times by making stronger assumptions about object lifetimes and sizes.
