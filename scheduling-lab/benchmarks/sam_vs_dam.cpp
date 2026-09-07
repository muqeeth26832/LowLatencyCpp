// SAM vs DAM work-allocation benchmark.
//
// Both methods scan the same range of integers and report the self-dividing
// ones. The serial case (m = 1) is included as the floor each parallel
// scheme is measured against, to show ideal vs achieved speedup.
//
// *BENCHMARKS can only run one config per binary invocation via args.

#include <benchmark/benchmark.h>

#include <cstdint>

#include "self_dividing.h"

static constexpr long long N    = 10'000'000;
static constexpr int       M    = 16;
static constexpr int       CHUNK = 256;

static void BM_Serial(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto r = run_sam(N, 1, CHUNK);
        benchmark::DoNotOptimize(r.numbers.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_SAM(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto r = run_sam(N, M, CHUNK);
        benchmark::DoNotOptimize(r.numbers.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_DAM(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto r = run_dam(N, M, CHUNK);
        benchmark::DoNotOptimize(r.numbers.size());
    }
    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_Serial)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_SAM)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_DAM)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();