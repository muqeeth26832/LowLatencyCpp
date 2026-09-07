// Multi-thread throughput.
//
//  - SPSC ring buffer is benchmarked under its intended 1P/1C shape:
//    one producer thread pushes N items while one consumer thread pops.
//  - LockQueue (mutex) and HWQueue (atomic slots) are MPMC: `thr`
//    producers push `total` items between them while `thr` consumers
//    drain all of them.
//
// HWQueue is run at a much smaller `total` because its O(tail) dequeue
// makes throughput collapse once the tail gets long; that is the point.

#include <benchmark/benchmark.h>

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

#include "hw_queue.h"
#include "lock_queue.h"
#include "spsc_ring_buffer.h"

// --------------------------------------------------
// 1P/1C for the SPSC ring buffer
// --------------------------------------------------

static void run_spsc(SPSCRingBuffer<std::uint64_t, 1 << 16>& q, std::size_t n)
{
    std::thread prod([&]() {
        for (std::size_t i = 0; i < n; ++i)
            while (!q.try_push(i)) { /* full: busy-wait */ }
    });

    std::thread cons([&]() {
        std::uint64_t v = 0;
        for (std::size_t i = 0; i < n; ++i) {
            while (!q.try_pop(v)) { /* empty: busy-wait */ }
            benchmark::DoNotOptimize(v);
        }
    });

    prod.join();
    cons.join();
}

static void BM_SPSC_Throughput(benchmark::State& state)
{
    const std::size_t n = state.range(0);

    for (auto _ : state)
    {
        SPSCRingBuffer<std::uint64_t, 1 << 16> q;
        run_spsc(q, n);
    }

    state.SetItemsProcessed(state.iterations() * n);
}

// --------------------------------------------------
// MPMC harness for LockQueue / HWQueue
// --------------------------------------------------

template <typename Q>
static void run_mpmc(Q& q, std::size_t prods, std::size_t cons, std::size_t total)
{
    std::vector<std::thread> threads;

    for (std::size_t p = 0; p < prods; ++p)
    {
        threads.emplace_back([&, p]() {
            const std::size_t share = total / prods;
            const std::size_t extra = (p == 0) ? (total % prods) : 0;
            for (std::size_t i = 0; i < share + extra; ++i) {
                while (!q.try_push(1)) std::this_thread::yield();
            }
        });
    }

    std::atomic<std::size_t> consumed{0};

    for (std::size_t c = 0; c < cons; ++c)
    {
        threads.emplace_back([&]() {
            std::uint64_t v = 0;
            while (consumed.load(std::memory_order_relaxed) < total)
            {
                if (q.try_pop(v))
                {
                    benchmark::DoNotOptimize(v);
                    consumed.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& t : threads) t.join();
}

template <typename Q>
static void BM_MPMC_Throughput(benchmark::State& state)
{
    const std::size_t total = state.range(0);
    const std::size_t thr   = state.range(1);

    for (auto _ : state)
    {
        Q q;
        run_mpmc(q, thr, thr, total);
    }

    state.SetItemsProcessed(state.iterations() * total);
}

using LockQueueQ = LockQueue<std::uint64_t, 1 << 20>;
using HWQueueQ   = HWQueue<std::uint64_t, 1 << 14>;

BENCHMARK(BM_SPSC_Throughput)->RangeMultiplier(4)->Range(1 << 16, 1 << 20);

BENCHMARK_TEMPLATE(BM_MPMC_Throughput, LockQueueQ)
    ->Args({1 << 20, 4})
    ->Args({1 << 20, 8});

BENCHMARK_TEMPLATE(BM_MPMC_Throughput, HWQueueQ)
    ->Args({1 << 13, 4})
    ->Args({1 << 13, 8});

BENCHMARK_MAIN();