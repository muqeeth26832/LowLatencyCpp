#pragma once
/*
Bitmask / bitboard active set.

Activeness lives in a dense array of uint64 words; bit k of word w means
instrument (w*64 + k) is active. Insert/erase are a single masked OR/ANDN.
Iteration uses countr_zero to walk only the set bits: O(active + N/64),
i.e. proportional to the number of active instruments, not the universe.
*/
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "instrument.h"

class BitmaskSet {
public:
    explicit BitmaskSet(std::vector<Instrument>& universe)
        : universe_(universe)
        , words_((universe.size() + 63) / 64, 0)
        , count_(0)
    {
    }

    void insert(std::uint32_t id)
    {
        std::size_t w = id >> 6;
        std::uint64_t mask = std::uint64_t{1} << (id & 63);
        if ((words_[w] & mask) == 0)
        {
            words_[w] |= mask;
            ++count_;
        }
    }

    void erase(std::uint32_t id)
    {
        std::size_t w = id >> 6;
        std::uint64_t mask = std::uint64_t{1} << (id & 63);
        if ((words_[w] & mask) != 0)
        {
            words_[w] &= ~mask;
            --count_;
        }
    }

    bool contains(std::uint32_t id) const
    {
        std::size_t w = id >> 6;
        return (words_[w] & (std::uint64_t{1} << (id & 63))) != 0;
    }

    std::size_t size() const { return count_; }

    void clear()
    {
        std::fill(words_.begin(), words_.end(), 0);
        count_ = 0;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::size_t w = 0; w < words_.size(); ++w)
        {
            std::uint64_t bits = words_[w];
            while (bits != 0)
            {
                std::size_t k = static_cast<std::size_t>(std::countr_zero(bits));
                fn(universe_[w * 64 + k]);
                bits &= bits - 1;
            }
        }
    }

private:
    std::vector<Instrument>& universe_;
    std::vector<std::uint64_t> words_;
    std::size_t count_;
};