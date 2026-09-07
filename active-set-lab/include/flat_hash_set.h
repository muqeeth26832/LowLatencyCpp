#pragma once
/*
Flat open-addressing hash set (the poor-man's absl::flat_hash_set).

Keys are stored inline in one contiguous power-of-two table with LINEAR
probing -- no linked buckets, no per-node allocation. This is what makes
Swiss-style tables cache-friendly for lookups versus std::unordered_set.

Two caveats that keep it honest vs the dense prefix sets:
  1. The table is sized for load factor <= 0.5, so it is ~2N slots.
  2. for_each is O(table capacity), not O(active): iteration must skim past
     empty and tombstone slots. absl speeds this up with SIMD word
     comparisons; we do scalar skips here, which is the pessimistic bound.

Sentinel encoding:
  kEmpty      = UINT32_MAX
  kTombstone  = UINT32_MAX-1
Real instrument ids are < 4096, so both sentinels are unambiguous.
*/
#include <cstdint>
#include <cstring>
#include <vector>

#include "instrument.h"

class FlatHashSet {
public:
    explicit FlatHashSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , table_(8, kEmpty)
        , count_(0)
        , tombs_(0)
    {
    }

    void insert(std::uint32_t id)
    {
        grow_if_needed();

        std::size_t mask = table_.size() - 1;
        std::size_t slot = hash(id) & mask;
        while (true)
        {
            std::uint32_t& cell = table_[slot];
            if (cell == kEmpty || cell == kTombstone || cell == id)
            {
                if (cell == kTombstone)
                    --tombs_;
                if (cell != id)
                {
                    cell = id;
                    ++count_;
                }
                return;
            }
            slot = (slot + 1) & mask;
        }
    }

    void erase(std::uint32_t id)
    {
        std::size_t mask = table_.size() - 1;
        std::size_t slot = hash(id) & mask;
        while (true)
        {
            std::uint32_t cell = table_[slot];
            if (cell == kEmpty)
                return;
            if (cell == id)
            {
                table_[slot] = kTombstone;
                --count_;
                ++tombs_;
                return;
            }
            slot = (slot + 1) & mask;
        }
    }

    bool contains(std::uint32_t id) const
    {
        std::size_t mask = table_.size() - 1;
        std::size_t slot = hash(id) & mask;
        while (true)
        {
            std::uint32_t cell = table_[slot];
            if (cell == kEmpty)
                return false;
            if (cell == id)
                return true;
            slot = (slot + 1) & mask;
        }
    }

    std::size_t size() const { return count_; }

    void clear()
    {
        std::fill(table_.begin(), table_.end(), kEmpty);
        count_ = 0;
        tombs_ = 0;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::uint32_t key : table_)
            if (key != kEmpty && key != kTombstone)
                fn(universe_[key]);
    }

private:
    static constexpr std::uint32_t kEmpty = UINT32_MAX;
    static constexpr std::uint32_t kTombstone = UINT32_MAX - 1u;

    static std::size_t hash(std::uint32_t key) { return key * 2654435761u; }

    void grow_if_needed()
    {
        // rehash when live + tombstones reach half the table (load factor 0.5)
        if ((count_ + tombs_) * 2 < table_.size())
            return;

        std::vector<std::uint32_t> old = std::move(table_);
        table_.assign(old.size() * 2, kEmpty);
        count_ = 0;
        tombs_ = 0;

        for (std::uint32_t key : old)
            if (key != kEmpty && key != kTombstone)
                insert(key);
    }

    std::vector<Instrument>& universe_;
    std::vector<std::uint32_t> table_;
    std::size_t count_;
    std::size_t tombs_;
};