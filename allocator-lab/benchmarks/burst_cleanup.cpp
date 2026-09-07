#include <benchmark/benchmark.h>

#include <vector>
#include <cstdint>

#include "arena_allocator.h"
#include "memory_pool_allocator.h"
#include "slab_allocator.h"

static volatile std::uintptr_t sink{};

struct Order {
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
    uint64_t side;
};

constexpr std::size_t N = 50000;

static void BM_StdBurstLifetime(benchmark::State& state)
{
    for(auto _ : state)
    {
        std::vector<Order*> ptrs;
        ptrs.reserve(N);

        for(std::size_t i=0;i<N;++i)
        {
            auto* ptr = new Order{};
            sink ^= reinterpret_cast<std::uintptr_t>(ptr);
            ptrs.push_back(ptr);
        }

        for(auto* ptr : ptrs)
        {
            delete ptr;
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_ArenaBurstLifetime(benchmark::State& state)
{
    Arena arena(N * sizeof(Order));

    for(auto _ : state)
    {
        for(std::size_t i=0;i<N;++i)
        {
            auto* ptr = arena.construct<Order>();

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);
        }

        arena.reset();
    }

    state.SetItemsProcessed(state.iterations() * N);
}


static void BM_PoolBurstLifetime(benchmark::State& state)
{
    MemoryPool<Order> pool(N);

    for(auto _ : state)
    {
        std::vector<Order*> ptrs;
        ptrs.reserve(N);

        for(std::size_t i=0;i<N;++i)
        {
            auto* ptr = pool.allocate();

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);

            ptrs.push_back(ptr);
        }

        for(auto* ptr : ptrs)
        {
            pool.deallocate(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_SlabBurstLifetime(benchmark::State& state)
{
    SlabAllocator slab(N);

    for(auto _ : state)
    {
        std::vector<Order*> ptrs;
        ptrs.reserve(N);

        for(std::size_t i=0;i<N;++i)
        {
            auto* ptr = slab.allocate<Order>();

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);

            ptrs.push_back(ptr);
        }

        for(auto* ptr : ptrs)
        {
            slab.deallocate(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}



BENCHMARK(BM_StdBurstLifetime);
BENCHMARK(BM_ArenaBurstLifetime);
BENCHMARK(BM_PoolBurstLifetime);
BENCHMARK(BM_SlabBurstLifetime);

BENCHMARK_MAIN();
