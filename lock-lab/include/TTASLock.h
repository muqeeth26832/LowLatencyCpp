#pragma once

#include <atomic>

// Test-And-Test-And-Set (TTAS) spin lock.
// Phase 1 spins with plain loads on a shared cache line; Phase 2
// only issues an RMW once the flag is observed free. Eliminates most
// of the cache-line ping-pong that makes TAS pathological, at the
// cost of re-polling after every failed CAS.

class TTASLock {
public:
    TTASLock()
        : flag_(false)
    {}

    void lock()
    {
        while (true) {
            while (flag_.load(std::memory_order_relaxed)) {
                // spin on a shared-but-uncontended line
            }
            bool expected = false;
            if (flag_.compare_exchange_weak(
                    expected, true,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {
                return;
            }
        }
    }

    void unlock()
    {
        flag_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> flag_;
};