#pragma once
/*
Swap-and-pop ("dense prefix") active sets.

All three keep the active elements packed at the front of a contiguous
vector (an O(active)-iteration prefix). The trick is the slot back-pointer:
every active element records its own index, so erase swaps the last element
into the hole, updates the moved element's slot, and pops -- O(1), no scan.

Variants differ only in what they store and move:
  DenseIdSet      - moves 4-byte ids
  DensePtrSet     - moves 8-byte pointers, slot stored inside the payload
  DenseStructSet  - moves whole 48-byte structs by value
*/
#include <cstdint>
#include <utility>
#include <vector>

#include "instrument.h"

class DenseIdSet {
public:
    explicit DenseIdSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , slot_(universe.size(), kNoSlot)
    {
        ids_.reserve(universe.size());
    }

    void insert(std::uint32_t id)
    {
        if (slot_[id] != kNoSlot)
            return;
        slot_[id] = static_cast<std::uint32_t>(ids_.size());
        ids_.push_back(id);
    }

    void erase(std::uint32_t id)
    {
        std::uint32_t i = slot_[id];
        if (i == kNoSlot)
            return;

        std::uint32_t last = ids_.back();
        ids_[i] = last;
        slot_[last] = i;
        ids_.pop_back();
        slot_[id] = kNoSlot;
    }

    bool contains(std::uint32_t id) const { return slot_[id] != kNoSlot; }
    std::size_t size() const { return ids_.size(); }

    void clear()
    {
        ids_.clear();
        std::fill(slot_.begin(), slot_.end(), kNoSlot);
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::uint32_t id : ids_)
            fn(universe_[id]);
    }

private:
    std::vector<Instrument>& universe_;
    std::vector<std::uint32_t> ids_;
    std::vector<std::uint32_t> slot_;
};

class DensePtrSet {
public:
    explicit DensePtrSet(std::vector<Instrument>& universe)
        : universe_(universe)
    {
        ptrs_.reserve(universe.size());
    }

    void insert(std::uint32_t id)
    {
        Instrument& el = universe_[id];
        if (el.slot != kNoSlot)
            return;
        el.slot = static_cast<std::uint32_t>(ptrs_.size());
        ptrs_.push_back(&el);
    }

    void erase(std::uint32_t id)
    {
        Instrument& el = universe_[id];
        std::uint32_t i = el.slot;
        if (i == kNoSlot)
            return;

        Instrument* last = ptrs_.back();
        ptrs_[i] = last;
        last->slot = i;
        ptrs_.pop_back();
        el.slot = kNoSlot;
    }

    bool contains(std::uint32_t id) const { return universe_[id].slot != kNoSlot; }
    std::size_t size() const { return ptrs_.size(); }

    void clear()
    {
        for (Instrument* p : ptrs_)
            p->slot = kNoSlot;
        ptrs_.clear();
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (const Instrument* p : ptrs_)
            fn(*p);
    }

private:
    std::vector<Instrument>& universe_;
    std::vector<Instrument*> ptrs_;
};

class DenseStructSet {
public:
    explicit DenseStructSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , slot_(universe.size(), kNoSlot)
    {
        structs_.reserve(universe.size());
    }

    void insert(std::uint32_t id)
    {
        if (slot_[id] != kNoSlot)
            return;
        slot_[id] = static_cast<std::uint32_t>(structs_.size());
        structs_.push_back(universe_[id]);
    }

    void erase(std::uint32_t id)
    {
        std::uint32_t i = slot_[id];
        if (i == kNoSlot)
            return;

        Instrument last = structs_.back();
        structs_[i] = last;
        slot_[last.id] = i;
        structs_.pop_back();
        slot_[id] = kNoSlot;
    }

    bool contains(std::uint32_t id) const { return slot_[id] != kNoSlot; }
    std::size_t size() const { return structs_.size(); }

    void clear()
    {
        structs_.clear();
        std::fill(slot_.begin(), slot_.end(), kNoSlot);
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (const Instrument& el : structs_)
            fn(el);
    }

private:
    std::vector<Instrument>& universe_;
    std::vector<Instrument> structs_;
    std::vector<std::uint32_t> slot_;
};