# Register Lab

A **multi-reader single-writer (MRSW) regular register** (Herlihy–Shavit,
*Art of Multiprocessor Programming*, Ch. 4) implemented twice — plain
`bool` toggles vs. `std::atomic<bool>` toggles — and benchmarked for the
size-scan cost that dominates both.

A value in `[0, M]` is encoded as `M` toggles: indices `0..value-1` are
true, everything at/above `value` is false, and the reader returns the
index of the first false toggle. `read()` and `write()` are both O(M).

## Directory layout

```
include/           regular_register.h (header-only)
benchmarks/
register           part 1: read/write latency vs. bound M
                   part 2: N-reader scalability, mixed read/write
notes/             this document
```

## Build & run

```bash
cmake -B build && cmake --build build -j
./build/register
```

## Results

### Worst-case read (value = M-1, so every toggle is scanned), ns

`M` is the number of toggles.

| M      | bool    | atomic |
|--------|---------|--------|
| 256    | 783     | 258    |
| 1024   | 3049    | 732    |
| 4096   | 12420   | 2755   |
| 16384  | 49766   | 11167  |
| 65536  | 204290  | 45948  |

Linear in M, as expected. The atomic variant is *faster* for reads because
`std::vector<bool>` is a bit-packed bitset — extracting a bit costs more
than a byte-aligned atomic load. Even at `seq_cst`, a pure atomic load has
no RMW cost, and here the access order doesn't contend because it's a
single-threaded scan.

### Write, ns

| M      | bool    | atomic |
|--------|---------|--------|
| 256    | 964     | 2964   |
| 1024   | 3634    | 12111  |
| 4096   | 14588   | 48557  |
| 16384  | 57224   | 186585 |
| 65536  | 243379  | 793795 |

Also linear; here atomic is ~3.3× slower because every toggle store is a
`seq_cst` RMW-visible store, whereas the bitset path only touches one 64-bit
word per 64 toggles.

### Concurrent access (bound = 1024)

| Threads | read-only        | mixed (1 writer + N-1 readers) |
|---------|------------------|--------------------------------|
| 1       | 2.32 ns/read     | —                              |
| 2       | 1.69 ns/read     | 5.9 µs/op                      |
| 4       | 2.24 ns/read     | 3.2 µs/op                      |
| 8       | 2.67 ns/read     | 1.95 µs/op                     |
| 16      | 3.24 ns/read     | 1.21 µs/op                     |

Reads scale: 16 readers saturate the memory system (~89 G reads/s aggregate).
Adding one writer serializes every operation (the writer's two O(1024)
passes dominate), so aggregate throughput climbs only because the readers
overlap with a slow writer.

## Interpretation

1. The register's guarantee comes from the **ordering of the two write
   passes** (write `0..value-1` true before clearing the tail), which is
   what makes it *regular* — a read that overlaps a write gets either the
   old or the new value, never a mix.

2. Both variant's dominant cost is the **O(M) scan**, not the atomicity.
   This is why real register designs are bounded to a handful of bits and
   why `bool[]` vs `atomic[]` pales next to representation choice
   (bitset vs. byte).

3. The MRSW shape is the foundation that consensus objects, swap queues
   and eventually locks/queues chain up from — an MRSW register has
   consensus number 2.

## Takeaway

Atomicity is cheap; **linear scans are not**. The interesting engineering
is never "add atomics", it's arranging that the hot path touches one cache
line — which is exactly the lesson the SPSC ring in `queue-lab` encodes.