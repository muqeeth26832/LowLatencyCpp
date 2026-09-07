// Pure lock/unlock contention benchmark.
//
// Every thread executes the same lock-guarded critical section, which does
// negligible real work. Latency is therefore dominated by the lock itself:
// coherence traffic, spinning strategy, and serialized handoff.
//
// Benchmark 1 (no-arg): empty critical section — measures a single
//   acquire/release pair, or the "fixed cost" of the lock.
// Benchmark 2 (Arg = CS bytes): the CS writes to a per-thread scratch
//   buffer, giving the lock a realistic amount of work to protect. Queue
//   locks amortize their fixed overhead here.
//
// Threads are pinned one-per-core so contention is measured on the
// coherence fabric, not on scheduler artifacts.
//
// Usage:
//   ./contention                 # all locks, threads 1..16 (powers of 2)
//   ./contention --benchmark_filter=CLH

#include <benchmark/benchmark.h>

#include <cstdint>
#include <thread>
#include <vector>

#include <pthread.h>
#include <sched.h>

#include "CLHLock.h"
#include "MCSLock.h"
#include "MutexLock.h"
#include "TASLock.h"
#include "TTASLock.h"

void pin_to_core(std::size_t core)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(static_cast<int>(core % std::thread::hardware_concurrency()), &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

thread_local std::uint64_t cs_work{0};

template <typename Lock>
static void BM_LockContention(benchmark::State& state)
{
    // One shared lock instance across all threads of this benchmark.
    static Lock* lk = new Lock();

    pin_to_core(state.thread_index());

    for (auto _ : state)
    {
        lk->lock();
        benchmark::DoNotOptimize(cs_work += 1);
        lk->unlock();
    }

    state.SetItemsProcessed(state.iterations() * state.threads());
}

template <typename Lock>
static void BM_LockContentionWithCS(benchmark::State& state)
{
    static Lock* lk = new Lock();

    pin_to_core(state.thread_index());

    thread_local std::vector<std::uint64_t> scratch(
        state.range(0) / sizeof(std::uint64_t));

    for (auto _ : state)
    {
        lk->lock();
        for (auto& w : scratch) w += 1;
        benchmark::DoNotOptimize(scratch.data());
        lk->unlock();
    }

    state.SetItemsProcessed(state.iterations() * state.threads());
}

constexpr std::size_t CS_BYTES = 1024;

BENCHMARK_TEMPLATE(BM_LockContention, MutexLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockContention, TASLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockContention, TTASLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockContention, CLHLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockContention, MCSLock)->ThreadRange(1, 16);

BENCHMARK_TEMPLATE(BM_LockContentionWithCS, MutexLock)->ThreadRange(1, 16)->RangeMultiplier(4)->Arg(1024)->Arg(4096)->Arg(16384);
BENCHMARK_TEMPLATE(BM_LockContentionWithCS, TASLock)->ThreadRange(1, 16)->RangeMultiplier(4)->Arg(1024)->Arg(4096)->Arg(16384);
BENCHMARK_TEMPLATE(BM_LockContentionWithCS, TTASLock)->ThreadRange(1, 16)->RangeMultiplier(4)->Arg(1024)->Arg(4096)->Arg(16384);
BENCHMARK_TEMPLATE(BM_LockContentionWithCS, CLHLock)->ThreadRange(1, 16)->RangeMultiplier(4)->Arg(1024)->Arg(4096)->Arg(16384);
BENCHMARK_TEMPLATE(BM_LockContentionWithCS, MCSLock)->ThreadRange(1, 16)->RangeMultiplier(4)->Arg(1024)->Arg(4096)->Arg(16384);

BENCHMARK_MAIN();