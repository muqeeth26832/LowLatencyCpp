#pragma once

#include <atomic>

// Mellor-Crummey & Scott (MCS) queue lock.
//
// Explicit linked list of QNodes. Each waiter spins on its OWN node
// (local memory), which is NUMA-friendly, and the lock is handed off
// by the releaser writing to the successor's node directly.
//
//   lock()   – append own node to the tail; if a predecessor existed,
//              link it to us and spin on our own locked flag.
//   unlock() – if no successor, CAS tail back to null; otherwise clear
//              the successor's locked flag to pass the lock.

class MCSLock {
private:
    struct QNode {
        std::atomic<bool>   locked;
        std::atomic<QNode*> next;
        QNode()
            : locked(false), next(nullptr)
        {}
    };

    static thread_local QNode my_node_;

    std::atomic<QNode*> tail_;

public:
    MCSLock()
        : tail_(nullptr)
    {}

    void lock()
    {
        QNode* node = &my_node_;
        node->locked.store(true, std::memory_order_relaxed);
        node->next.store(nullptr, std::memory_order_relaxed);

        QNode* pred = tail_.exchange(node, std::memory_order_acq_rel);

        if (pred != nullptr) {
            pred->next.store(node, std::memory_order_release);
            while (node->locked.load(std::memory_order_acquire)) {
                // spin on own node
            }
        }
    }

    void unlock()
    {
        QNode* node = &my_node_;

        QNode* succ = node->next.load(std::memory_order_acquire);
        if (succ == nullptr) {
            QNode* expected = node;
            if (tail_.compare_exchange_strong(
                    expected, nullptr,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                return;
            }
            while ((succ = node->next.load(std::memory_order_acquire)) == nullptr) {
                // a successor is about to link itself; wait for it
            }
        }
        succ->locked.store(false, std::memory_order_release);
    }
};

inline thread_local MCSLock::QNode MCSLock::my_node_;