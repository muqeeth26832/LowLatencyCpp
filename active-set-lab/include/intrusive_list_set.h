#pragma once
/*
Intrusive doubly-linked list active set.

No container storage at all: activeness IS membership in a doubly-linked
list threaded through the payload's prev/next fields. Insert links the
node at the head (O(1)); erase unlinks via the node's own prev/next
(O(1), no back-pointer map needed). Iteration follows next pointers, so
cost is O(active) but each step is a dependent pointer load.

When inactive, prev == next == self (self-loop sentinel), which makes
contains() a single pointer compare.
*/
#include <cstdint>
#include <vector>

#include "instrument.h"

class IntrusiveListSet {
public:
    explicit IntrusiveListSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , head_(nullptr)
        , count_(0)
    {
    }

    void insert(std::uint32_t id)
    {
        Instrument& el = universe_[id];
        if (el.next != &el)
            return; // already in the list

        el.prev = nullptr;
        el.next = head_;
        if (head_ != nullptr)
            head_->prev = &el;
        head_ = &el;
        ++count_;
    }

    void erase(std::uint32_t id)
    {
        Instrument& el = universe_[id];
        if (el.next == &el)
            return; // not in the list

        if (el.prev != nullptr)
            el.prev->next = el.next;
        else
            head_ = el.next;
        if (el.next != nullptr)
            el.next->prev = el.prev;

        el.prev = &el;
        el.next = &el;
        --count_;
    }

    bool contains(std::uint32_t id) const { return universe_[id].next != &universe_[id]; }

    std::size_t size() const { return count_; }

    void clear()
    {
        Instrument* cur = head_;
        while (cur != nullptr)
        {
            Instrument* next = cur->next;
            cur->prev = cur;
            cur->next = cur;
            cur = next;
        }
        head_ = nullptr;
        count_ = 0;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (const Instrument* cur = head_; cur != nullptr; cur = cur->next)
            fn(*cur);
    }

private:
    std::vector<Instrument>& universe_;
    Instrument* head_;
    std::size_t count_;
};