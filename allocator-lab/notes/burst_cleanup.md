## Burst Lifetime Benchmark

Workload:

```cpp
allocate N objects

destroy all objects together
```

where:

```cpp
N = 50,000
```

This benchmark models workloads where many objects share the same lifetime and can be released in a single batch.

Examples include:

- Market data processing batches
- Order matching cycles
- Temporary parser objects
- Simulation timesteps

### Results

| Allocator | Time (ns) | Throughput |
|------------|------------:|------------:|
| `std::new/delete` | 4,379,987 | 11.4 M ops/s |
| Arena | 773,446 | 64.6 M ops/s |
| Memory Pool | 1,606,352 | 31.1 M ops/s |
| Slab Allocator | 1,889,285 | 26.5 M ops/s |

### Relative Speedup vs `std::new/delete`

| Allocator | Speedup |
|------------|------------:|
| Arena | **5.7×** |
| Memory Pool | **2.7×** |
| Slab Allocator | **2.3×** |

### Interpretation

- **Arena** performs best because cleanup is a constant-time operation:

  ```cpp
  arena.reset();
  ```

  Instead of freeing thousands of individual objects, the allocator simply resets its internal offset.

- **Memory Pool** must return every object back to the free list individually, resulting in O(N) cleanup cost.

- **Slab Allocator** exhibits similar behavior to the memory pool but incurs additional overhead from managing size classes.

- **`std::new/delete`** performs the most work because every object must be individually returned to the heap allocator.

### Comparison with Previous Benchmarks

| Allocator | Allocation Only | Alloc + Free | Burst Lifetime |
|------------|------------:|------------:|------------:|
| `std::new/delete` | ~85 ns | ~33 ns | ~88 ns |
| Arena | ~16 ns | N/A | ~15 ns |
| Memory Pool | ~40 ns | ~8 ns | ~32 ns |
| Slab | ~47 ns | ~17 ns | ~38 ns |

The burst-lifetime workload highlights the primary strength of arena allocators: when many objects share the same lifetime, deallocation can be reduced to a single reset operation rather than thousands of individual frees.
