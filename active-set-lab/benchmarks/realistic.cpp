#include <benchmark/benchmark.h>

#include <cstdint>
#include <random>
#include <vector>

#include "bench_utils.h"
#include "bitmask_set.h"
#include "dense_active_sets.h"
#include "flag_baselines.h"
#include "flat_hash_set.h"
#include "instrument.h"
#include "intrusive_list_set.h"
#include "stl_baselines.h"

namespace {
constexpr std::size_t kUniverseSize = 4096;
constexpr std::uint32_t kSeed = 1234u;

// The money benchmark: each "tick" churns `frac`% of the active instruments
// (inserts + erases) and then sweeps the whole active set. Half the universe
// is active, so churn is frac% of that half.
template <typename Set>
static void BM_RealisticTick(benchmark::State& state)
{
    const std::size_t half = kUniverseSize / 2;
    const std::size_t churn = half * state.range(0) / 100;

    auto universe = make_universe(kUniverseSize);
    Set set(universe);

    std::vector<std::uint8_t> ref(kUniverseSize, 0);
    std::vector<std::uint32_t> ids = shuffled_ids(kUniverseSize, kSeed);
    for (std::size_t i = 0; i < half; ++i)
    {
        set.insert(ids[i]);
        ref[ids[i]] = 1;
    }

    std::mt19937 rng(42);
    volatile std::uint64_t sink = 0;
    for (auto _ : state)
    {
        for (std::size_t i = 0; i < churn; ++i)
        {
            std::uint32_t id = static_cast<std::uint32_t>(rng() % kUniverseSize);
            if (ref[id])
            {
                set.erase(id);
                ref[id] = 0;
            }
            else
            {
                set.insert(id);
                ref[id] = 1;
            }
        }
        set.for_each([&](const Instrument& it) { sink += it.quantity; });
    }

    state.SetItemsProcessed(state.iterations() * (churn + half));
    state.SetLabel("churn:" + std::to_string(state.range(0)) + "%");
}
} // namespace

#define REG(T) BENCHMARK_TEMPLATE(BM_RealisticTick, T)->Arg(10)->Arg(50)->Arg(90)
REG(TreeSet);
REG(HashSet);
REG(ByteFlagSet);
REG(BitFlagSet);
REG(BitmaskSet);
REG(DenseIdSet);
REG(DensePtrSet);
REG(DenseStructSet);
REG(IntrusiveListSet);
REG(FlatHashSet);
#undef REG

BENCHMARK_MAIN();