## Random Lifetime Benchmark

Workload:

```text
60% allocate
40% deallocate random live object
```

Parameters:

```cpp
OPERATIONS       = 1,000,000
MAX_LIVE_OBJECTS = 50,000
```

This benchmark simulates a long-running system where objects are continuously created and destroyed with varying lifetimes.

Examples include:

- Order entry and cancellation in trading systems
- Request handling in servers
- Entity management in games
- General application heaps

Unlike the previous benchmarks, objects do not share the same lifetime and are freed in a random order.

### Results

| Allocator | Time (ns) | Throughput |
|------------|------------:|------------:|
| `std::new/delete` | 65,613,884 | 15.2 M ops/s |
| Memory Pool | 54,284,590 | 18.4 M ops/s |
| Slab Allocator | 58,560,623 | 17.1 M ops/s |

### Relative Speedup vs `std::new/delete`

| Allocator | Speedup |
|------------|------------:|
| Memory Pool | **1.21×** |
| Slab Allocator | **1.12×** |

### Interpretation

- **Memory Pool** remains the fastest allocator because allocations and deallocations are simple free-list operations with no heap interaction.
- **Slab Allocator** performs similarly, but incurs a small amount of overhead when selecting the appropriate size class.
- **`std::new/delete`** is slower due to heap management and allocator bookkeeping.

### Why the Gap Is Smaller

Compared to previous benchmarks, the performance differences are significantly reduced:

| Benchmark | Best Speedup |
|------------|------------:|
| Allocation Only | ~5.2× |
| Alloc + Free | ~3.9× |
| Burst Lifetime | ~5.7× |
| Random Lifetime | ~1.2× |

The random lifetime workload spends a substantial amount of time managing the live object set and performing random removals. As a result, allocation itself becomes a smaller fraction of the total runtime, reducing the advantage of specialized allocators.

### Key Takeaway

Arena allocators dominate workloads with shared lifetimes, while memory pools excel when objects are frequently allocated and freed individually. Under realistic mixed-lifetime workloads, specialized allocators still outperform the standard heap allocator, but the improvement is much smaller than in synthetic allocation-heavy benchmarks.
