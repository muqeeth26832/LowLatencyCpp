#pragma once

#include <cstdint>
#include <list>

// Shared workload for the locked-list benchmark.
// Not thread-safe by itself; a lock is provided externally.

struct SharedList {
    std::list<int> data;

    bool lookup(int val) const
    {
        for (auto& v : data) if (v == val) return true;
        return false;
    }

    void insert(int val) { data.push_back(val); }
    void remove(int val) { data.remove(val); }
};