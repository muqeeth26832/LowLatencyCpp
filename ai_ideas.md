For a repo called **Building Blocks for LowLatencyCPP**, I'd focus on things that demonstrate:

1. **Performance awareness**
2. **Systems knowledge**
3. **Modern C++**
4. **Trading/low-latency relevance**

rather than reimplementing every STL container.

---

# High Priority

These are the strongest resume/interview topics.

## Memory

### Allocator Lab ✅

Already done.

---

### Cache Lab

Show effects of:

```cpp
vector
vs
linked list
```

Measure:

* traversal time
* cache misses (perf)

Great demonstration of locality.

---

### False Sharing Lab

```cpp
struct Counter {
    std::atomic<int> x;
};
```

vs

```cpp
struct alignas(64) Counter {
    std::atomic<int> x;
};
```

Benchmark.

Huge interview topic.

---

### Data-Oriented Design

Compare:

```cpp
struct Order {
    ...
};
std::vector<Order>
```

vs

```cpp
struct Orders {
    std::vector<int> price;
    std::vector<int> qty;
};
```

(AoS vs SoA)

---

# Concurrency

Extremely important for quant firms.

## SPSC Queue

Single Producer Single Consumer

Ring buffer.

Classic interview topic.

---

## MPSC Queue

Multi Producer Single Consumer.

More advanced.

---

## Lock Benchmark Lab

Compare:

```cpp
std::mutex
std::shared_mutex
spinlock
```

under contention.

---

## Thread Pool

Simple implementation.

Good systems project.

---

# Networking

Very valuable.

## TCP vs UDP Lab

Measure:

* throughput
* latency

for small messages.

---

## Market Data Feed Simulator

UDP multicast style feed.

Very quant relevant.

---

## Exchange Gateway

Simple TCP server:

```text
NEW_ORDER
CANCEL_ORDER
MODIFY_ORDER
```

---

# Modern C++

## Perfect Forwarding

Small examples.

---

## Move Semantics Lab

Benchmark:

```cpp
copy
move
```

for large objects.

---

## Smart Pointer Lab

Implement:

```cpp
UniquePtr
```

Maybe:

```cpp
SharedPtr
```

but only if you really want to understand atomics.

---

# CPU Architecture

These impress interviewers.

## Branch Prediction Lab

```cpp
if(random)
```

vs

```cpp
if(always_true)
```

Measure.

---

## TLB Lab

Access memory:

```cpp
contiguous
```

vs

```cpp
page hopping
```

Measure.

---

## Prefetching Lab

Use:

```cpp
__builtin_prefetch
```

and compare.

---

# Trading Specific

This is what Edelweiss, Graviton, Quadeye, NK, Tower, etc. care about.

## Order Book

Must have.

---

## Matching Engine

Price-time priority.

---

## Risk Gateway

```text
max position
max order size
kill switch
```

---

## Market Data Publisher

Generate book updates.

---

## Exchange Simulator

Tie together:

```text
Gateway
OrderBook
Risk
MarketData
```

This becomes a flagship project.

---

# Benchmarking / Profiling

## Perf Lab

Use:

```bash
perf stat
perf record
```

Show:

* cache misses
* branch misses
* IPC

---

## Google Benchmark Lab

Already started.

---

# Lower Priority

These are educational but less impactful.

* Linked list
* Vector implementation
* Hash map implementation
* Doubly linked list
* LRU cache

Good learning exercises, but they don't stand out as much as concurrency, cache, networking, and trading systems.

---

If I were optimizing specifically for **quant/low-latency interviews**, I'd build this roadmap:

```text
Allocator Lab                 ✅
Cache Lab
False Sharing Lab
SPSC Queue
Lock Benchmark Lab
TCP vs UDP
Order Book
Matching Engine
Risk Gateway
Market Data Publisher
Exchange Simulator
```

That sequence alone can easily become a very impressive systems-performance portfolio.
