#pragma once

#include <mutex>

// std::mutex wrapper. Included as the baseline that the spin locks
// are measured against. The kernel may block a waiter and hand off
// the lock to the next runnable thread; the cost is a syscall-ish
// wakeup path instead of a bounded spin.

class MutexLock {
public:
    void lock() { mtx_.lock(); }
    void unlock() { mtx_.unlock(); }

private:
    std::mutex mtx_;
};