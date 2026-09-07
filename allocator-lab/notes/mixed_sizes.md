## Mixed Object Sizes Benchmark

Workload:

Randomly allocate **100,000 objects** from four size classes:

```cpp
struct Small  { std::byte data[24];  };
struct Medium { std::byte data[60];  };
struct Large  { std::byte data[120]; };
struct Huge   { std::byte data[400]; };
```

Each object type is chosen with equal probability.

This benchmark evaluates allocator performance when handling varying allocation sizes rather than repeatedly allocating a single object type.

### Results

| Allocator | Time (ns) | Throughput |
|------------|------------:|------------:|
| `std::new` | 30,825,075 | 3.24 M allocs/s |
| Arena | 5,611,334 | 17.82 M allocs/s |
| Slab Allocator | 13,272,942 | 7.53 M allocs/s |

### Relative Speedup vs `std::new`

| Allocator | Speedup |
|------------|------------:|
| Arena | **5.49×** |
| Slab Allocator | **2.32×** |

### Interpretation

- **Arena Allocator** achieves the best performance because allocation remains a simple bump-pointer operation regardless of object size. Allocation cost is effectively independent of whether the object is 24 bytes or 400 bytes.
- **Slab Allocator** performs substantially better than the standard allocator by routing allocations into fixed-size pools (32, 64, 128, and 512-byte slabs), avoiding repeated heap allocations.
- **`std::new`** is the slowest because each allocation requires interaction with the general-purpose heap allocator, which must handle arbitrary allocation sizes and maintain allocator metadata.

### Why Slab Performs Better Here

In previous benchmarks, every object was exactly 32 bytes:

```cpp
struct Order {
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
    uint64_t side;
};
```

As a result, the slab allocator effectively behaved as:

```text
Slab -> 32-byte Pool
```

with little opportunity to demonstrate the benefits of multiple size classes.

In this benchmark:

```text
24 B  -> 32-byte slab
60 B  -> 64-byte slab
120 B -> 128-byte slab
400 B -> 512-byte slab
```

The allocator now utilizes all slab classes, making the workload representative of the scenarios slab allocators are designed to handle.

### Summary of All Benchmarks

| Benchmark | Arena | Pool | Slab |
|------------|------------:|------------:|------------:|
| Allocation Only | ~5.2× | ~2.1× | ~1.8× |
| Alloc + Free | N/A | ~3.9× | ~2.0× |
| Burst Lifetime | ~5.7× | ~2.7× | ~2.3× |
| Random Lifetime | N/A | ~1.3× | ~1.2× |
| Mixed Sizes | ~5.5× | N/A | ~2.3× |

### Key Takeaway

The benchmarks highlight the workloads each allocator is designed for:

- **Arena Allocators** excel when many objects share a common lifetime and can be released together.
- **Memory Pools** excel when repeatedly allocating and freeing objects of the same size.
- **Slab Allocators** provide a compromise, supporting multiple object sizes while retaining much of the performance benefit of fixed-size pools.
- **General-purpose allocators** offer maximum flexibility but incur additional overhead to support arbitrary allocation patterns.
