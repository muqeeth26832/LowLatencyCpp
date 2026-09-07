#include <benchmark/benchmark.h>

#include <vector>
#include <random>
#include <cstdint>

#include "memory_pool_allocator.h"
#include "slab_allocator.h"

static volatile std::uintptr_t sink{};

struct Order {
    uint64_t order_id;
    uint64_t price;
    uint64_t quantity;
    uint64_t side;
};

constexpr std::size_t OPERATIONS       = 1'000'000;
constexpr std::size_t MAX_LIVE_OBJECTS = 50'000;

struct Operation {
    bool allocate;
    std::size_t index;
};

static std::vector<Operation> generate_workload()
{
    std::mt19937 rng(42);

    std::vector<Operation> ops;
    ops.reserve(OPERATIONS);

    std::size_t live_count = 0;

    for (std::size_t i = 0; i < OPERATIONS; ++i)
    {
        bool do_allocate =
            live_count == 0 ||
            (live_count < MAX_LIVE_OBJECTS && (rng() % 100) < 60);

        if (do_allocate)
        {
            ops.push_back({true, 0});
            ++live_count;
        }
        else
        {
            ops.push_back({false, rng() % live_count});
            --live_count;
        }
    }

    return ops;
}

static const auto workload = generate_workload();


// --------------------------------------------------
// std::new / delete
// --------------------------------------------------

static void BM_StdRandomLifetime(benchmark::State& state)
{
    for (auto _ : state)
    {
        state.PauseTiming();

        std::vector<Order*> live;
        live.reserve(MAX_LIVE_OBJECTS);

        state.ResumeTiming();

        for (const auto& op : workload)
        {
            if (op.allocate)
            {
                auto* ptr = new Order{};

                sink ^= reinterpret_cast<std::uintptr_t>(ptr);

                live.push_back(ptr);
            }
            else
            {
                auto idx = op.index % live.size();

                delete live[idx];

                live[idx] = live.back();
                live.pop_back();
            }
        }

        state.PauseTiming();

        for (auto* ptr : live)
        {
            delete ptr;
        }

        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * OPERATIONS);
}


// --------------------------------------------------
// Memory Pool
// --------------------------------------------------

static void BM_PoolRandomLifetime(benchmark::State& state)
{
    MemoryPool<Order> pool(MAX_LIVE_OBJECTS);

    for (auto _ : state)
    {
        state.PauseTiming();

        std::vector<Order*> live;
        live.reserve(MAX_LIVE_OBJECTS);

        state.ResumeTiming();

        for (const auto& op : workload)
        {
            if (op.allocate)
            {
                auto* ptr = pool.allocate();

                sink ^= reinterpret_cast<std::uintptr_t>(ptr);

                live.push_back(ptr);
            }
            else
            {
                auto idx = op.index % live.size();

                pool.deallocate(live[idx]);

                live[idx] = live.back();
                live.pop_back();
            }
        }

        state.PauseTiming();

        for (auto* ptr : live)
        {
            pool.deallocate(ptr);
        }

        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * OPERATIONS);
}


// --------------------------------------------------
// Slab
// --------------------------------------------------

static void BM_SlabRandomLifetime(benchmark::State& state)
{
    SlabAllocator slab(MAX_LIVE_OBJECTS);

    for (auto _ : state)
    {
        state.PauseTiming();

        std::vector<Order*> live;
        live.reserve(MAX_LIVE_OBJECTS);

        state.ResumeTiming();

        for (const auto& op : workload)
        {
            if (op.allocate)
            {
                auto* ptr = slab.allocate<Order>();

                sink ^= reinterpret_cast<std::uintptr_t>(ptr);

                live.push_back(ptr);
            }
            else
            {
                auto idx = op.index % live.size();

                slab.deallocate(live[idx]);

                live[idx] = live.back();
                live.pop_back();
            }
        }

        state.PauseTiming();

        for (auto* ptr : live)
        {
            slab.deallocate(ptr);
        }

        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * OPERATIONS);
}


// --------------------------------------------------

BENCHMARK(BM_StdRandomLifetime);
BENCHMARK(BM_PoolRandomLifetime);
BENCHMARK(BM_SlabRandomLifetime);

BENCHMARK_MAIN();
