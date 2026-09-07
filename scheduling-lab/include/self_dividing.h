#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

// Static vs. dynamic work allocation for a CPU-bound scan (self-dividing
// numbers).
//
// SAM (static allocation method): threads are pre-assigned interleaved
// chunk-sized ranges; nobody touches work that belongs to another thread.
// DAM (dynamic allocation method): all threads pull the next chunk from a
// single shared atomic counter, so a fast thread can absorb a slow one's
// residual work at the cost of an atomic fetch_add per chunk.

inline bool is_self_dividing(int x)
{
    int temp = x;
    while (temp > 0)
    {
        int d = temp % 10;
        if (d == 0 || x % d != 0) return false;
        temp /= 10;
    }
    return true;
}

struct WorkResult {
    std::vector<int> numbers;
};

inline WorkResult run_sam(long long N, int m, int chunk)
{
    std::vector<std::vector<int>> per_thread(m);
    std::vector<std::thread> threads;

    const long long stride = static_cast<long long>(m) * chunk;

    for (int tid = 0; tid < m; ++tid)
    {
        threads.emplace_back([&, tid]() {
            long long base = static_cast<long long>(tid) * chunk + 1;
            while (base < N)
            {
                long long end = std::min(base + chunk, N);
                for (long long num = base; num < end; ++num)
                {
                    if (is_self_dividing(static_cast<int>(num)))
                        per_thread[tid].push_back(static_cast<int>(num));
                }
                base += stride;
            }
        });
    }

    for (auto& t : threads) t.join();

    WorkResult result;
    for (auto& v : per_thread)
        result.numbers.insert(result.numbers.end(), v.begin(), v.end());
    return result;
}

inline WorkResult run_dam(long long N, int m, int chunk)
{
    std::vector<std::vector<int>> per_thread(m);
    std::vector<std::thread> threads;

    std::atomic<long long> counter{1};

    for (int tid = 0; tid < m; ++tid)
    {
        threads.emplace_back([&, tid]() {
            while (true)
            {
                long long base = counter.fetch_add(chunk, std::memory_order_relaxed);
                if (base >= N) break;

                long long end = std::min(base + chunk, N);
                for (long long num = base; num < end; ++num)
                {
                    if (is_self_dividing(static_cast<int>(num)))
                        per_thread[tid].push_back(static_cast<int>(num));
                }
            }
        });
    }

    for (auto& t : threads) t.join();

    WorkResult result;
    for (auto& v : per_thread)
        result.numbers.insert(result.numbers.end(), v.begin(), v.end());
    return result;
}