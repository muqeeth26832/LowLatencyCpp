#pragma once

#include <cstddef>
#include <memory>
#include <mutex>

// Mutex-protected FIFO ring buffer. The baseline every lock-free queue is
// measured against. All operations are O(1), but every enqueue/dequeue
// pays lock/unlock overhead, and producers contend on a single mutex.

template <typename T, std::size_t CAPACITY>
class LockQueue {
public:
    static_assert((CAPACITY & (CAPACITY - 1)) == 0,
                  "CAPACITY must be a power of two");

    LockQueue()
        : slots_(new T[CAPACITY])
    {}

    bool try_push(const T& value)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (count_ == CAPACITY) return false;

        slots_[tail_] = value;
        tail_ = (tail_ + 1) & (CAPACITY - 1);
        ++count_;
        return true;
    }

    bool try_pop(T& out)
    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (count_ == 0) return false;

        out = slots_[head_];
        head_ = (head_ + 1) & (CAPACITY - 1);
        --count_;
        return true;
    }

private:
    mutable std::mutex mtx_;
    std::unique_ptr<T[]> slots_;
    std::size_t         head_ = 0;
    std::size_t         tail_ = 0;
    std::size_t         count_ = 0;
};