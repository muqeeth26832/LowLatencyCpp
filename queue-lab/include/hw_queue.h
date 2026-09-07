#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

// Herlihy-Wing (1991) non-blocking queue.
//
// A fixed-size array of atomic slots plus a shared tail counter. Producers
// claim a slot with fetch_add on tail and install a heap-allocated node;
// consumers scan from index 0 with a timed exchange (nullptr) and take the
// first non-null slot.
//
// No locks and bounded wait-free producer progress, but:
//  - every enqueue pays +1 heap allocation,
//  - dequeue is O(tail): it re-scans already-consumed slots,
//  - slots are never reused, so the tail only grows (capacity bound).
//
// Included as the "non-blocking" design to contrast with a real
// cache-friendly lock-free ring buffer.

template <typename T, std::size_t CAPACITY>
class HWQueue {
public:
    HWQueue()
        : slots_(CAPACITY)
    {
        for (auto& s : slots_) s.store(nullptr, std::memory_order_relaxed);
    }

    HWQueue(const HWQueue&) = delete;
    HWQueue& operator=(const HWQueue&) = delete;

    ~HWQueue()
    {
        for (auto& s : slots_) {
            if (T* p = s.load(std::memory_order_relaxed)) delete p;
        }
    }

    bool try_push(const T& value)
    {
        const std::size_t i = tail_.fetch_add(1, std::memory_order_acq_rel);
        if (i >= CAPACITY) {
            tail_.fetch_sub(1, std::memory_order_relaxed);
            return false;
        }
        slots_[i].store(new T(value), std::memory_order_release);
        return true;
    }

    bool try_pop(T& out)
    {
        const std::size_t range = tail_.load(std::memory_order_acquire);
        for (std::size_t i = 0; i < range; ++i) {
            T* p = slots_[i].exchange(nullptr, std::memory_order_acq_rel);
            if (p != nullptr) {
                out = *p;
                delete p;
                return true;
            }
        }
        return false;
    }

private:
    alignas(64) std::atomic<std::size_t> tail_{0};
    std::vector<std::atomic<T*>>         slots_;
};