#include <benchmark/benchmark.h>

#include <cstdint>
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

template <typename Set>
static void BM_IterateActives(benchmark::State& state)
{
    const int frac = static_cast<int>(state.range(0)); // active fraction %
    const std::size_t active = kUniverseSize * frac / 100;

    auto universe = make_universe(kUniverseSize);
    Set set(universe);

    std::vector<std::uint32_t> ids = shuffled_ids(kUniverseSize, kSeed);
    for (std::size_t i = 0; i < active; ++i)
        set.insert(ids[i]);

    volatile std::uint64_t sink = 0;
    for (auto _ : state)
    {
        set.for_each([&](const Instrument& it) { sink += it.quantity; });
    }

    state.SetItemsProcessed(state.iterations() * active);
    state.SetLabel("active:" + std::to_string(frac) + "%");
}
} // namespace

#define REG(T) BENCHMARK_TEMPLATE(BM_IterateActives, T)->Arg(10)->Arg(25)->Arg(50)->Arg(75)->Arg(100)
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