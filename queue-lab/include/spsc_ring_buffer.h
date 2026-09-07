#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <utility>

// Single-producer single-consumer lock-free ring buffer (Vyukov-style).
//
// The producer only writes the tail index; the consumer only writes the
// head index. Each index lives on its own cache line so pushes and pops
// never ping-pong a shared line. Enqueue/disqueue are O(1) with at most
// one atomic store + one relaxed/acquire load each.
//
// Memory ordering:
//   try_push: load head acquire (consumer's release-store of head),
//             store slot, store tail release.
//   try_pop : load tail acquire (producer's release-store of tail),
//             load slot, store head release.
//
// T must be copy/move assignable. Capacity must be a power of two.

template <typename T, std::size_t CAPACITY>
class SPSCRingBuffer {
public:
    static_assert((CAPACITY & (CAPACITY - 1)) == 0,
                  "CAPACITY must be a power of two");

    SPSCRingBuffer() = default;

    SPSCRingBuffer(const SPSCRingBuffer&) = delete;
    SPSCRingBuffer& operator=(const SPSCRingBuffer&) = delete;

    bool try_push(const T& value)
    {
        const std::size_t t = tail_.load(std::memory_order_relaxed);
        const std::size_t h = head_.load(std::memory_order_acquire);
        if (t - h == CAPACITY) return false;   // full

        slots_[t & (CAPACITY - 1)] = value;
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    bool try_push(T&& value)
    {
        const std::size_t t = tail_.load(std::memory_order_relaxed);
        const std::size_t h = head_.load(std::memory_order_acquire);
        if (t - h == CAPACITY) return false;

        slots_[t & (CAPACITY - 1)] = std::move(value);
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    bool try_pop(T& out)
    {
        const std::size_t t = tail_.load(std::memory_order_acquire);
        const std::size_t h = head_.load(std::memory_order_relaxed);
        if (t == h) return false;               // empty

        out = slots_[h & (CAPACITY - 1)];
        head_.store(h + 1, std::memory_order_release);
        return true;
    }

private:
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
    alignas(64) std::array<T, CAPACITY>  slots_{};
};