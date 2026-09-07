## Building Blocks for LowLatencyCPP

This repo is where you can find the right tool for the trade.
All about LowLatencyCPP header-only code which one can use based on their needs.

Each lab is self-contained: `include/` for header-only implementations,
`benchmarks/` for Google Benchmark sources, `notes/` for measured results
and interpretation.

## Build everything

```bash
cmake -B build && cmake --build build -j
```

## Current modules

### `allocator-lab`

Arena, slab and memory-pool allocators vs. `new`/`delete` across allocation,
alloc+free, burst, mixed-size and random-lifetime workloads.

### `lock-lab`

TAS, TTAS, CLH and MCS locks plus a `std::mutex` baseline — empty-CS
contention, CS-size scaling, and a shared-list workload. Shows TAS's RMW
storm, TTAS's fix, and when futex-based blocking beats spinning.

### `queue-lab`

Mutex ring buffer, Herlihy-Wing atomic queue, and a cache-line-padded SPSC
ring buffer. Single-thread latency plus multi-thread throughput; SPSC ends
up ~24× faster than the mutex queue and orders of magnitude faster than the
"non-blocking" HW queue.

### `register-lab`

MRSW regular register (bool vs. atomic toggle arrays) — linear read/write
scan cost, read scalability, and the 1-writer+N-reader shape.

### `scheduling-lab`

Static (SAM) vs. dynamic (DAM) work allocation with an atomic chunk counter
over a CPU-bound scan — where the ~30% dynamic wins.

### `active-set-lab`

9 ways to track which instruments are active and iterate only them:
`std::set`/`unordered_set` baselines, byte/bit flag sweeps, bitmask (bitboard),
and swap-and-pop dense sets (id, pointer, by-value) plus an intrusive list
and a flat open-addressing hash set.
Six workloads: sparse/dense iteration, insert-only, remove-only, bulk
lifecycle, pure churn, and a realistic churn+sweep tick. Shows the core
trade-off: dense prefix = O(active) sequential iteration but per-op movement;
flags bitmask = O(1) mutation but O(N) scan; node containers lose everywhere.

## Roadmap

- Low-latency queues: lock-free MPSC/MPMC variants
- Cache/false-sharing labs (vector vs. linked list, `alignas(64)`)
- Networking: TCP vs. UDP latency, market-data feed shapes

The `PCP_assgs/` directory holds the original course assignments these
labs were distilled from.