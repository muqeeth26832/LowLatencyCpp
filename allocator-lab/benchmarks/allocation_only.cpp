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
}; // 32 bytes

// ----------------------------------------------------
// std::new
// ----------------------------------------------------

static void BM_StdAllocation(benchmark::State& state)
{
    constexpr std::size_t N = 100000;

    for (auto _ : state)
    {
        std::vector<Order*> ptrs;
        ptrs.reserve(N);

        for (std::size_t i = 0; i < N; ++i)
        {
            auto* ptr = new Order{};

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);

            ptrs.push_back(ptr);
        }

        for (auto* ptr : ptrs)
        {
            delete ptr;
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}

// ----------------------------------------------------
// Arena
// ----------------------------------------------------

static void BM_ArenaAllocation(benchmark::State& state)
{
    constexpr std::size_t N = 100000;

    Arena arena(N * sizeof(Order));

    for (auto _ : state)
    {
        arena.reset();

        for (std::size_t i = 0; i < N; ++i)
        {
            auto* ptr = arena.construct<Order>();

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}

// ----------------------------------------------------
// Memory Pool
// ----------------------------------------------------

static void BM_PoolAllocation(benchmark::State& state)
{
    constexpr std::size_t N = 100000;

    MemoryPool<Order> pool(N);

    for (auto _ : state)
    {
        std::vector<Order*> ptrs;
        ptrs.reserve(N);

        for (std::size_t i = 0; i < N; ++i)
        {
            auto* ptr = pool.allocate();

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);

            ptrs.push_back(ptr);
        }

        for (auto* ptr : ptrs)
        {
            pool.deallocate(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}

// ----------------------------------------------------
// Slab
// ----------------------------------------------------

static void BM_SlabAllocation(benchmark::State& state)
{
    constexpr std::size_t N = 100000;

    SlabAllocator slab(N);

    for (auto _ : state)
    {
        std::vector<Order*> ptrs;
        ptrs.reserve(N);

        for (std::size_t i = 0; i < N; ++i)
        {
            auto* ptr = slab.allocate<Order>();

            sink ^= reinterpret_cast<std::uintptr_t>(ptr);

            ptrs.push_back(ptr);
        }

        for (auto* ptr : ptrs)
        {
            slab.deallocate(ptr);
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}

// ----------------------------------------------------

BENCHMARK(BM_StdAllocation);
BENCHMARK(BM_ArenaAllocation);
BENCHMARK(BM_PoolAllocation);
BENCHMARK(BM_SlabAllocation);

BENCHMARK_MAIN();
