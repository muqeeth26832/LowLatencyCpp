# Scheduling Lab

Two work-distribution strategies for a CPU-bound scan (counting
self-dividing numbers in `[1, N]`), run on `M` threads with chunk-sized
work items:

| Strategy | How work is split |
|----------|-------------------|
| **SAM** — static allocation | Each thread is pre-assigned interleaved `chunk`-sized ranges; no sharing, zero coordination |
| **DAM** — dynamic allocation | All threads pull the next `chunk` from a single shared atomic counter (`fetch_add`) |

Serial (`M=1`) is the floor that both are measured against.

## Directory layout

```
include/           self_dividing.h (header-only)
benchmarks/
sam_vs_dam         serial / SAM / DAM over a fixed range
notes/             this document
```

## Build & run

```bash
cmake -B build && cmake --build build -j
./build/sam_vs_dam
```

## Results

Range `N = 10,000,000`, `M = 16` threads, `chunk = 256`.

| Scheme      | Wall time | Speedup vs serial |
|-------------|-----------|------------------|
| Serial      | 141 ms    | 1.0×              |
| SAM (static) | 26.0 ms  | 5.4×              |
| DAM (dynamic) | 18.2 ms | 7.7×              |

DAM beats SAM by ~30%. The range scan is uniform, so both schemes trawl
the same amount of work — the difference is entirely the residual-balance
effect: with a shared atomic counter, a thread that falls behind (or a core
that is briefly contended) has its unworked chunk pulled by whoever is
free, whereas SAM's interleaving irrevocably assigns ranges up front.

## Cost of the shared counter

DAM's dynamism is not free: every chunk grab is an atomic `fetch_add` on a
line shared by all threads. That cost is tiny relative to a `chunk=256`
work item (~hundreds of `is_self_dividing` calls), so DAM pays almost
nothing for load balancing here.

## When static wins

If chunks were *expensive to compute* and *finish close together* (the
uniform case), the fetch_add cache-line ping-pong starts to matter and SAM
can pull ahead. Static allocation also wins when placement matters
(cache-affine ranges) or when the coordination cost can't be hidden.

## Takeaway

Dynamic allocation is a one-line change that buys most of the work-stealing
benefit without a scheduler — a good default for uniform CPU-bound scans,
and the seed of the same reasoning that later justifies real work-stealing
queues.