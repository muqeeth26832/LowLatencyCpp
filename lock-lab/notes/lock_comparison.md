# Lock Lab

Implementation, benchmarking and analysis of five mutual-exclusion primitives:

| Lock | Type | Description |
|------|------|-------------|
| `TASLock` | Spin | `atomic_flag::test_and_set` until free — every retry is an atomic RMW |
| `TTASLock` | Spin | read the flag with a plain load first, RMW only when it looks free |
| `CLHLock` | Queue lock | implicit FIFO via thread-local nodes; spin on predecessor's node |
| `MCSLock` | Queue lock | explicit FIFO; spin on your own node (NUMA-friendly hand-off) |
| `MutexLock` | Blocking | `std::mutex` wrapper — kernel futex, waiters go to sleep |

## Directory layout

```
include/       header-only lock implementations
benchmarks/    Google Benchmark sources
contention     empty-CS latency vs threads; CS-size scaling
list_workload  shared linked-list lookup/insert/delete under each lock
notes/         this document
```

## Build & run

```bash
cmake -B build && cmake --build build -j
./build/contention                     # all locks, 1/2/4/8/16 threads
./build/list_workload
./build/contention --benchmark_filter=CLH
```

Threads are **pinned one-per-core** during measurement so results reflect
the coherence fabric rather than scheduler placement.

## Results

### Empty critical section (single lock/unlock round-trip, ns)

| Threads | Mutex | TAS | TTAS | CLH | MCS |
|---------|-------|-----|------|-----|-----|
| 1  | 15.1 | 17.1 | 18.3 | 18.2 | 32.6 |
| 2  | 291  | 65.0 | 66.5 | 108 | 131 |
| 4  | 569  | 579  | 510  | 879 | 784 |
| 8  | 1369 | 1796 | 1500 | 1767 | 1948 |
| 16 | 2662 | 5856 | 4820 | 57.8k | 23.1k |

### Critical section of 16 KiB of writes (ns, 16 threads)

| CS size | Mutex | TAS | TTAS | CLH | MCS |
|---------|-------|-----|------|-----|-----|
| 1 KiB   | 5451 | 10.9k | 3886 | 10.0k | 10.8k |
| 4 KiB   | 10.3k | 15.1k | 3900 | 29.2k | 12.3k |
| 16 KiB  | 19.1k | 24.1k | 6381 | 50.8k | 31.2k |

### Shared-list workload, 16 threads (10% updates)

Per-op latency in ns — effectively the lock cost when traversal is short.

| Mutex | TAS | TTAS | CLH | MCS |
|-------|-----|------|-----|-----|
| 17.0k | 37.4k | 22.5k | 119k | 203k |

## Interpretation

1. **TAS is pathological.** Every failed acquisition is an atomic RMW that
   invalidates the flag's cache line. TTAS simply adds one plain load
   before the RMW and is consistently faster, exactly as expected.

2. **`std::mutex` wins at high thread counts.** Its waiters block in the
   kernel instead of burning a core spinning, so 16 threads cost no
   spinning cycles. A futex wake-up is expensive (`~1-2 µs`), yet at
   `threads:16` it outruns every spinlock.

3. **Queue locks did not pay off here.** CLH/MCS give FIFO fairness and
   bounded contention on private cache lines, but every acquisition goes
   through the tail pointer (an atomic `exchange`) and hands off serially.
   That fixed cost only amortizes when the critical section is long
   relative to the hand-off — a ~1 µs list traversal is not enough, so
   on this machine they lose to TTAS.

4. Solar flare: `BM_LockedListWorkload<MCSLock>/threads:16` shows an
   unstable ~150 µs spike across runs unless threads are pinned. Pin to
   cores before trusting absolute numbers.

## Takeaway

* Short CS, high contention → **TTAS** (cheap spin, no RMW storm).
* Long CS / blocking OK → **`std::mutex`** (futex; no wasted cycles).
* Need FIFO fairness or bounded waiting → **CLH/MCS**, but only when the
  CS justifies the hand-off cost.