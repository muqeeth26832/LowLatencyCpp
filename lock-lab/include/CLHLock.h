#pragma once

#include <atomic>

// Craig-Landin-Hagersten (CLH) queue lock.
//
// Threads form an implicit FIFO via a chain of QNodes. Each waiter spins
// on its predecessor's node, so all spinning is on a private cache line
// instead of a global flag. Acquisition and release are O(1).
//
//   lock()   – push own node (locked=true) with an atomic exchange on the
//              tail, then spin on the returned predecessor until it frees.
//   unlock() – clear own node so the successor can proceed, then recycle
//              the predecessor's node for the next acquisition.

class CLHLock {
private:
    struct QNode {
        std::atomic<bool> locked;
        QNode()
            : locked(false)
        {}
    };

    static thread_local QNode* my_node_;
    static thread_local QNode* my_pred_;

    std::atomic<QNode*> tail_;

public:
    CLHLock()
    {
        QNode* sentinel = new QNode();
        sentinel->locked.store(false, std::memory_order_relaxed);
        tail_.store(sentinel, std::memory_order_relaxed);
    }

    ~CLHLock()
    {
        delete tail_.load();
    }

    void lock()
    {
        if (!my_node_) my_node_ = new QNode();
        my_node_->locked.store(true, std::memory_order_relaxed);

        my_pred_ = tail_.exchange(my_node_, std::memory_order_acq_rel);

        while (my_pred_->locked.load(std::memory_order_acquire)) {
            // spin on predecessor's node (private cache line)
        }
    }

    void unlock()
    {
        my_node_->locked.store(false, std::memory_order_release);

        my_node_ = my_pred_;
        my_pred_ = nullptr;
    }
};

inline thread_local CLHLock::QNode* CLHLock::my_node_ = nullptr;
inline thread_local CLHLock::QNode* CLHLock::my_pred_ = nullptr;