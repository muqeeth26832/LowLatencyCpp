// Locked shared-list workload.
//
// Port of the PCP spin-locks assignment workload: N threads run a mix of
// lookups (80%), inserts (10%) and deletes (10%) against one shared
// std::list, guarded by the lock under test. This measures the cost of
// a realistic critical section (cache-line traffic from list traversal)
// rather than a near-empty one.
//
//   ./list_workload --benchmark_filter=MCS

#include <benchmark/benchmark.h>

#include <cstdint>
#include <random>
#include <thread>

#include <pthread.h>
#include <sched.h>

#include "CLHLock.h"
#include "MCSLock.h"
#include "MutexLock.h"
#include "TASLock.h"
#include "TTASLock.h"
#include "shared_list.h"

void pin_to_core(std::size_t core)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(static_cast<int>(core % std::thread::hardware_concurrency()), &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

template <typename Lock>
static void BM_LockedListWorkload(benchmark::State& state)
{
    static Lock*       lk  = new Lock();
    static SharedList* lst = []() {
        auto* l = new SharedList();
        for (int i = 0; i < 1000; ++i) l->insert(i);
        return l;
    }();

    pin_to_core(state.thread_index());

    thread_local std::mt19937                    rng{std::random_device{}()};
    std::uniform_real_distribution<double>       coin(0.0, 1.0);
    std::uniform_int_distribution<int>           val(0, 999);

    for (auto _ : state)
    {
        double c = coin(rng);
        int    v = val(rng);

        lk->lock();
        if (c < 0.80) {
            lst->lookup(v);
        }
        else if (c < 0.90) {
            lst->insert(v);
        }
        else {
            lst->remove(v);
        }
        lk->unlock();
    }

    state.SetItemsProcessed(state.iterations() * state.threads());
}

BENCHMARK_TEMPLATE(BM_LockedListWorkload, MutexLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockedListWorkload, TASLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockedListWorkload, TTASLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockedListWorkload, CLHLock)->ThreadRange(1, 16);
BENCHMARK_TEMPLATE(BM_LockedListWorkload, MCSLock)->ThreadRange(1, 16);

BENCHMARK_MAIN();