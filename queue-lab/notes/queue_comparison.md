# Queue Lab

Three queue designs, measured for raw op latency and multi-threaded throughput:

| Queue | Concurrency | Notes |
|-------|-------------|-------|
| `LockQueue` | MPMC | O(1) push/pop behind one `std::mutex`; the baseline |
| `HWQueue`   | MPMC | Herlihy-Wing (1991): atomic slot array + shared tail; heap-allocates per enqueue, **O(tail) dequeue** |
| `SPSCRingBuffer` | SPSC | Vyukov-style lock-free ring; producer & consumer own separate cache-lines; 1 store + 2 loads per op |

## Directory layout

```
include/       header-only queue implementations
benchmarks/
single_thread  push/pop pair latency, one thread, zero contention
multi_thread   SPSC at 1P/1C; MPMC harness for LockQueue/HWQueue
notes/         this document
```

## Build & run

```bash
cmake -B build && cmake --build build -j
./build/single_thread
./build/multi_thread
```

## Results: single-thread push/pop pair latency

| Queue | pair latency |
|-------|--------------|
| `LockQueue` | 55.1 ns |
| `HWQueue`   | 111 µs |
| `SPSCRingBuffer` | 2.3 ns |

The SPSC ring is ~24× faster than the mutex queue and ~50,000× faster than
the HW queue with no contention. `HWQueue`'s dequeue scans every index up
to the tail on each pop, and allocates a heap node per push.

## Results: multi-threaded throughput

| Configuration | Items | Time |
|---------------|-------|------|
| `SPSC` 1 producer + 1 consumer | 65 536 pairs | 1.13 ms (~440 M pairs/s) |
| `SPSC` 1 producer + 1 consumer | 1 048 576 pairs | 22 ms (~5.8 G pairs/s) |
| `LockQueue` 4 prod + 4 cons | 1 048 576 ops | 732 ms (~1.4 M ops/s) |
| `LockQueue` 8 prod + 8 cons | 1 048 576 ops | 1013 ms (~1.0 M ops/s) |
| `HWQueue` 4 prod + 4 cons | 8192 ops | 155 ms (~53 k ops/s) |
| `HWQueue` 8 prod + 8 cons | 8192 ops | 105 ms (~78 k ops/s) |

`HWQueue` was run at 8192 ops: its optimism bubble bursts above that.
`LockQueue` plateaus with thread count because every op serializes on the
same mutex (no producer can make progress while the lock is contended).

## Interpretation

1. **SPSC is the right shape for max throughput.** With one writer and one
   reader there is exactly one shared pair of indices, each owned by a
   different thread and cache-line-padded against false sharing. No mutex,
   no CAS loop — ops are a store to a ring slot plus a release store on the
   boundary index.

2. **MPMC is where the hardware bites.** Any MPMC design must serialize
   producers (a mutex, or an atomic `fetch_add` on a shared counter) and
   consumers. The Herlihy-Wing queue is lock-free in name only here: the
   linear dequeue scan makes it 2-3 orders of magnitude slower, which is
   precisely why production low-latency systems hand one queue to one
   consumer instead of sharing it.

3. **Blocking vs. lock-free trade-off.** The mutex queue's kernel-based
   blocking is *predictable*, but a producer pausing a scheduler quantum
   can stall the whole line at tail latency. The SPSC ring never blocks
   past the current operation.

## Takeaway

* Dedicated SPSC ring is the fastest, cheapest-to-reason-about design.
* `HWQueue` is an instructive museum piece — it's on this board to show
  that "non-blocking" is not the same as "fast".
* For a shared queue, a mutex often beats poorly-optimised lock-free
  code; the win comes from *avoiding* sharing, not from atomics.