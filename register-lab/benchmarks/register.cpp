// MRSW regular register benchmark.
//
// Part 1 – single-thread read/write latency vs register bound M. Both read
//   and write fan out over the M boolean toggles, so latency should scale
//   linearly with M.
// Part 2 – real MRSW scaling: N reader threads hammer read() while (in the
//   mixed case) one thread writes, to show how a shared register behaves
//   on the coherence fabric.

#include <benchmark/benchmark.h>

#include <random>

#include "regular_register.h"

// --------------------------------------------------
// Part 1: single-thread, bound scaling
// --------------------------------------------------

static void BM_RegisterRead_Bool(benchmark::State& state)
{
    const std::size_t M = state.range(0);
    RegularRegister reg(M, /*atomic_mode=*/false);
    // Worst-case read: a full O(M) scan for the first false toggle.
    reg.write(static_cast<int>(M - 1));

    for (auto _ : state) benchmark::DoNotOptimize(reg.read());
    state.SetItemsProcessed(state.iterations());
}

static void BM_RegisterRead_Atomic(benchmark::State& state)
{
    const std::size_t M = state.range(0);
    RegularRegister reg(M, /*atomic_mode=*/true);
    reg.write(static_cast<int>(M - 1));

    for (auto _ : state) benchmark::DoNotOptimize(reg.read());
    state.SetItemsProcessed(state.iterations());
}

static void BM_RegisterWrite_Bool(benchmark::State& state)
{
    const std::size_t M = state.range(0);
    RegularRegister reg(M, /*atomic_mode=*/false);
    std::mt19937 rng(42);

    for (auto _ : state) {
        reg.write(static_cast<int>(rng() % M));
    }
    state.SetItemsProcessed(state.iterations());
}

static void BM_RegisterWrite_Atomic(benchmark::State& state)
{
    const std::size_t M = state.range(0);
    RegularRegister reg(M, /*atomic_mode=*/true);
    std::mt19937 rng(42);

    for (auto _ : state) {
        reg.write(static_cast<int>(rng() % M));
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_RegisterRead_Bool)->RangeMultiplier(4)->Range(1 << 8, 1 << 16);
BENCHMARK(BM_RegisterRead_Atomic)->RangeMultiplier(4)->Range(1 << 8, 1 << 16);
BENCHMARK(BM_RegisterWrite_Bool)->RangeMultiplier(4)->Range(1 << 8, 1 << 16);
BENCHMARK(BM_RegisterWrite_Atomic)->RangeMultiplier(4)->Range(1 << 8, 1 << 16);

// --------------------------------------------------
// Part 2: concurrent MRSW access
// --------------------------------------------------

// Shared register across all threads of a benchmark run.
constexpr std::size_t REG_BOUND = 1024;
static RegularRegister* shared_reg() {
    static RegularRegister* reg = new RegularRegister(REG_BOUND, true);
    return reg;
}

static void BM_MRSW_ReadScalability(benchmark::State& state)
{
    RegularRegister* reg = shared_reg();

    for (auto _ : state) benchmark::DoNotOptimize(reg->read());
    state.SetItemsProcessed(state.iterations() * state.threads());
}

static void BM_MRSW_MixedReadWrite(benchmark::State& state)
{
    RegularRegister* reg = shared_reg();
    thread_local std::mt19937 rng{std::random_device{}()};

    for (auto _ : state) {
        if (state.thread_index() == 0) {
            reg->write(static_cast<int>(rng() % REG_BOUND));
        }
        else {
            benchmark::DoNotOptimize(reg->read());
        }
    }
    state.SetItemsProcessed(state.iterations() * state.threads());
}

BENCHMARK(BM_MRSW_ReadScalability)->ThreadRange(1, 16);
BENCHMARK(BM_MRSW_MixedReadWrite)->ThreadRange(2, 16);

BENCHMARK_MAIN();