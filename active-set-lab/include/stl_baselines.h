#pragma once
/*
STL baselines: std::set (red-black tree) and std::unordered_set (separate chaining).
Both store ids. for_each walks the container and maps each id back into the
universe array.
*/
#include <cstdint>
#include <set>
#include <unordered_set>

#include "instrument.h"

class TreeSet {
public:
    explicit TreeSet(std::vector<Instrument>& universe)
        : universe_(universe)
    {
    }

    void insert(std::uint32_t id) { set_.insert(id); }
    void erase(std::uint32_t id) { set_.erase(id); }
    bool contains(std::uint32_t id) const { return set_.count(id) != 0; }
    std::size_t size() const { return set_.size(); }

    void clear()
    {
        set_.clear();
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::uint32_t id : set_)
            fn(universe_[id]);
    }

private:
    std::vector<Instrument>& universe_;
    std::set<std::uint32_t> set_;
};

class HashSet {
public:
    explicit HashSet(std::vector<Instrument>& universe)
        : universe_(universe)
    {
        set_.reserve(universe.size());
    }

    void insert(std::uint32_t id) { set_.insert(id); }
    void erase(std::uint32_t id) { set_.erase(id); }
    bool contains(std::uint32_t id) const { return set_.count(id) != 0; }
    std::size_t size() const { return set_.size(); }

    void clear()
    {
        set_.clear();
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        for (std::uint32_t id : set_)
            fn(universe_[id]);
    }

private:
    std::vector<Instrument>& universe_;
    std::unordered_set<std::uint32_t> set_;
};