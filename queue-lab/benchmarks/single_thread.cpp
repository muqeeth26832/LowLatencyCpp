// Single-thread push/pop latency.
//
// One thread alternates push/pop against each queue. This isolates the
// constant cost of one enqueue + one dequeue with zero cross-thread
// contention: the mutex baseline, the non-blocking Herlihy-Wing queue,
// and the SPSC ring buffer.
//
// Note: HWQueue dequeue is O(tail) (it re-scans consumed slots), so its
// iteration count is kept small to bound total work.

#include <benchmark/benchmark.h>

#include <cstdint>
#include <utility>

#include "hw_queue.h"
#include "lock_queue.h"
#include "spsc_ring_buffer.h"

template <typename Q>
static void BM_SingleThreadPushPop(benchmark::State& state)
{
    Q q;

    std::uint64_t v = 0;
    for (auto _ : state)
    {
        while (!q.try_push(1)) { /* full */ }

        if (!q.try_pop(v)) state.SkipWithError("pop failed");
        benchmark::DoNotOptimize(v);
    }

    state.SetItemsProcessed(state.iterations() * 2);
}

using LockQueueQ = LockQueue<std::uint64_t, 1 << 16>;
using HWQueueQ   = HWQueue<std::uint64_t, 1 << 22>;
using SPSCQueueQ = SPSCRingBuffer<std::uint64_t, 1 << 16>;

BENCHMARK_TEMPLATE(BM_SingleThreadPushPop, LockQueueQ)->Iterations(1 << 18);
BENCHMARK_TEMPLATE(BM_SingleThreadPushPop, HWQueueQ)->Iterations(1 << 14);
BENCHMARK_TEMPLATE(BM_SingleThreadPushPop, SPSCQueueQ)->Iterations(1 << 18);

BENCHMARK_MAIN();