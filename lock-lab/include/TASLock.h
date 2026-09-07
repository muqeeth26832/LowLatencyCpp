#pragma once

#include <atomic>

// Test-And-Set (TAS) spin lock.
// A thread spins issuing test_and_set until it acquires the lock.
// Simple, but every RMW invalidates the cache line under contention,
// hammering the coherence fabric with bus traffic.

class TASLock {
public:
    TASLock()
        : flag_(ATOMIC_FLAG_INIT)
    {}

    void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // spin
        }
    }

    void unlock()
    {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_;
};