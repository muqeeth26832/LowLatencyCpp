#include <benchmark/benchmark.h>

#include <array>
#include <random>
#include <vector>
#include <cstddef>
#include <cstdint>

#include "arena_allocator.h"
#include "slab_allocator.h"

static volatile std::uintptr_t sink{};

struct Small  { std::byte data[24];  };
struct Medium { std::byte data[60];  };
struct Large  { std::byte data[120]; };
struct Huge   { std::byte data[400]; };

constexpr std::size_t N = 100000;

enum class Type : uint8_t {
    Small,
    Medium,
    Large,
    Huge
};

static std::vector<Type> generate_workload()
{
    std::mt19937 rng(42);

    std::vector<Type> workload;
    workload.reserve(N);

    for(std::size_t i = 0; i < N; ++i)
    {
        workload.push_back(
            static_cast<Type>(rng() % 4)
        );
    }

    return workload;
}

static const auto workload = generate_workload();


// --------------------------------------------------
// std::new
// --------------------------------------------------

static void BM_StdMixedSizes(benchmark::State& state)
{
    for(auto _ : state)
    {
        std::vector<void*> ptrs;
        ptrs.reserve(N);

        for(auto type : workload)
        {
            switch(type)
            {
                case Type::Small:
                {
                    auto* ptr = new Small{};
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }

                case Type::Medium:
                {
                    auto* ptr = new Medium{};
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }

                case Type::Large:
                {
                    auto* ptr = new Large{};
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }

                case Type::Huge:
                {
                    auto* ptr = new Huge{};
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }
            }
        }

        state.PauseTiming();

        for(std::size_t i = 0; i < N; ++i)
        {
            switch(workload[i])
            {
                case Type::Small:
                    delete static_cast<Small*>(ptrs[i]);
                    break;

                case Type::Medium:
                    delete static_cast<Medium*>(ptrs[i]);
                    break;

                case Type::Large:
                    delete static_cast<Large*>(ptrs[i]);
                    break;

                case Type::Huge:
                    delete static_cast<Huge*>(ptrs[i]);
                    break;
            }
        }

        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * N);
}


// --------------------------------------------------
// Arena
// --------------------------------------------------

static void BM_ArenaMixedSizes(benchmark::State& state)
{
    constexpr std::size_t ARENA_SIZE =
        N * sizeof(Huge);

    Arena arena(ARENA_SIZE);

    for(auto _ : state)
    {
        arena.reset();

        for(auto type : workload)
        {
            switch(type)
            {
                case Type::Small:
                {
                    auto* ptr = arena.construct<Small>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    break;
                }

                case Type::Medium:
                {
                    auto* ptr = arena.construct<Medium>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    break;
                }

                case Type::Large:
                {
                    auto* ptr = arena.construct<Large>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    break;
                }

                case Type::Huge:
                {
                    auto* ptr = arena.construct<Huge>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    break;
                }
            }
        }
    }

    state.SetItemsProcessed(state.iterations() * N);
}


// --------------------------------------------------
// Slab
// --------------------------------------------------

static void BM_SlabMixedSizes(benchmark::State& state)
{
    SlabAllocator slab(N);

    for(auto _ : state)
    {
        std::vector<void*> ptrs;
        ptrs.reserve(N);

        for(auto type : workload)
        {
            switch(type)
            {
                case Type::Small:
                {
                    auto* ptr = slab.allocate<Small>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }

                case Type::Medium:
                {
                    auto* ptr = slab.allocate<Medium>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }

                case Type::Large:
                {
                    auto* ptr = slab.allocate<Large>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }

                case Type::Huge:
                {
                    auto* ptr = slab.allocate<Huge>();
                    sink ^= reinterpret_cast<std::uintptr_t>(ptr);
                    ptrs.push_back(ptr);
                    break;
                }
            }
        }

        state.PauseTiming();

        for(std::size_t i = 0; i < N; ++i)
        {
            switch(workload[i])
            {
                case Type::Small:
                    slab.deallocate(static_cast<Small*>(ptrs[i]));
                    break;

                case Type::Medium:
                    slab.deallocate(static_cast<Medium*>(ptrs[i]));
                    break;

                case Type::Large:
                    slab.deallocate(static_cast<Large*>(ptrs[i]));
                    break;

                case Type::Huge:
                    slab.deallocate(static_cast<Huge*>(ptrs[i]));
                    break;
            }
        }

        state.ResumeTiming();
    }

    state.SetItemsProcessed(state.iterations() * N);
}


BENCHMARK(BM_StdMixedSizes);
BENCHMARK(BM_ArenaMixedSizes);
BENCHMARK(BM_SlabMixedSizes);

BENCHMARK_MAIN();
