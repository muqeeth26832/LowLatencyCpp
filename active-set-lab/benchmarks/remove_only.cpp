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
static void BM_RemoveOnly(benchmark::State& state)
{
    auto universe = make_universe(kUniverseSize);
    std::vector<std::uint32_t> ids = shuffled_ids(kUniverseSize, kSeed);

    for (auto _ : state)
    {
        Set set(universe);
        for (std::uint32_t id : ids)
            set.insert(id);

        for (std::uint32_t id : ids)
            set.erase(id);
        benchmark::DoNotOptimize(set.size());
    }

    state.SetItemsProcessed(state.iterations() * kUniverseSize);
}
} // namespace

#define REG(T) BENCHMARK_TEMPLATE(BM_RemoveOnly, T)
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