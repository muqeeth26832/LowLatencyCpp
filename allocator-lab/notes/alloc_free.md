## Allocation + Deallocation Benchmark

Workload:

```cpp
auto* ptr = allocator.allocate();
allocator.deallocate(ptr);
```

This benchmark measures the cost of allocating and immediately returning a single object to the allocator.

### Results

| Allocator | Latency | Throughput |
|------------|------------:|------------:|
| `std::new/delete` | 32.8 ns | 30.5 M ops/s |
| Memory Pool | 8.43 ns | 118.6 M ops/s |
| Slab Allocator | 16.6 ns | 60.2 M ops/s |

### Relative Speedup vs `std::new/delete`

| Allocator | Speedup |
|------------|------------:|
| Memory Pool | **3.9×** |
| Slab Allocator | **2.0×** |

### Interpretation

- **Memory Pool** performs best because allocation and deallocation are both simple free-list operations. No heap bookkeeping or system allocator interaction is required.
- **Slab Allocator** is built on top of memory pools but incurs additional overhead from selecting the appropriate size class before allocating.
- **`std::new/delete`** remains the most flexible solution but pays for that flexibility with additional allocator metadata management and heap synchronization costs.

### Comparison with Allocation-Only Benchmark

| Allocator | Allocation Only | Alloc + Free |
|------------|------------:|------------:|
| `std::new` | ~85 ns | ~33 ns |
| Memory Pool | ~40 ns | ~8 ns |
| Slab | ~47 ns | ~17 ns |

The alloc/free benchmark is faster because it repeatedly reuses the same memory blocks. Once the allocator's free lists are populated, allocations become highly cache-friendly and avoid most of the work required when continuously requesting fresh memory.
