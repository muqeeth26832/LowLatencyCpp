#include <benchmark/benchmark.h>

#include <cstdint>

#include "memory_pool_allocator.h"
#include "slab_allocator.h"

static volatile std::uintptr_t sink{};

struct Order {
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
    uint64_t side;
}; // 32 bytes

// --------------------------------------------------
// std::new / delete
// --------------------------------------------------

static void BM_StdAllocFree(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto* ptr = new Order{};

        sink ^= reinterpret_cast<std::uintptr_t>(ptr);

        delete ptr;
    }

    state.SetItemsProcessed(state.iterations());
}

// --------------------------------------------------
// Memory Pool
// --------------------------------------------------

static void BM_PoolAllocFree(benchmark::State& state)
{
    constexpr std::size_t N = 1024;

    MemoryPool<Order> pool(N);

    for (auto _ : state)
    {
        auto* ptr = pool.allocate();

        sink ^= reinterpret_cast<std::uintptr_t>(ptr);

        pool.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
}

// --------------------------------------------------
// Slab
// --------------------------------------------------

static void BM_SlabAllocFree(benchmark::State& state)
{
    constexpr std::size_t N = 1024;

    SlabAllocator slab(N);

    for (auto _ : state)
    {
        auto* ptr = slab.allocate<Order>();

        sink ^= reinterpret_cast<std::uintptr_t>(ptr);

        slab.deallocate(ptr);
    }

    state.SetItemsProcessed(state.iterations());
}

// --------------------------------------------------

BENCHMARK(BM_StdAllocFree);
BENCHMARK(BM_PoolAllocFree);
BENCHMARK(BM_SlabAllocFree);

BENCHMARK_MAIN();
