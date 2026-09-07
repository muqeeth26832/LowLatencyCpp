#pragma once
/*
Shared benchmark helpers: deterministic id shuffles so every
implementation exercises an identical operation sequence.
*/
#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

inline std::vector<std::uint32_t> shuffled_ids(std::size_t n, std::uint32_t seed = 1234u)
{
    std::vector<std::uint32_t> ids(n);
    for (std::size_t i = 0; i < n; ++i)
        ids[i] = static_cast<std::uint32_t>(i);

    std::mt19937 rng(seed);
    std::shuffle(ids.begin(), ids.end(), rng);
    return ids;
}