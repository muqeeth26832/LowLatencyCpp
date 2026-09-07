#pragma once
/*
Flag-sweep baselines: a byte flag array and a bit-packed flag array.

Every read/write is O(1), but iterating the active set costs O(N):
for_each must test every flag slot regardless of how few are set.
*/
#include <cstdint>
#include <vector>

#include "instrument.h"

class ByteFlagSet {
public:
    explicit ByteFlagSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , flags_(universe.size(), 0)
        , count_(0)
    {
    }

    void insert(std::uint32_t id)
    {
        if (flags_[id] == 0)
        {
            flags_[id] = 1;
            ++count_;
        }
    }

    void erase(std::uint32_t id)
    {
        if (flags_[id] != 0)
        {
            flags_[id] = 0;
            --count_;
        }
    }

    bool contains(std::uint32_t id) const { return flags_[id] != 0; }
    std::size_t size() const { return count_; }

    void clear()
    {
        std::fill(flags_.begin(), flags_.end(), 0);
        count_ = 0;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::size_t i = 0; i < flags_.size(); ++i)
            if (flags_[i] != 0)
                fn(universe_[i]);
    }

private:
    std::vector<Instrument>& universe_;
    std::vector<std::uint8_t> flags_;
    std::size_t count_;
};

class BitFlagSet {
public:
    explicit BitFlagSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , flags_(universe.size(), false)
        , count_(0)
    {
    }

    void insert(std::uint32_t id)
    {
        if (!flags_[id])
        {
            flags_[id] = true;
            ++count_;
        }
    }

    void erase(std::uint32_t id)
    {
        if (flags_[id])
        {
            flags_[id] = false;
            --count_;
        }
    }

    bool contains(std::uint32_t id) const { return flags_[id]; }
    std::size_t size() const { return count_; }

    void clear()
    {
        std::fill(flags_.begin(), flags_.end(), false);
        count_ = 0;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::size_t i = 0; i < flags_.size(); ++i)
            if (flags_[i])
                fn(universe_[i]);
    }

private:
    std::vector<Instrument>& universe_;
    std::vector<bool> flags_;
    std::size_t count_;
};