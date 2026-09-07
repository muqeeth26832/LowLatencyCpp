#pragma once
/*
Common payload types shared by every active-set implementation.

The "universe" is a pre-allocated, stable array of all instruments
keyed by id. It is created once and never reallocated, so intrusive
and pointer-based sets can safely hold addresses into it.
*/
#include <cstdint>
#include <vector>

struct Instrument {
    uint64_t price;
    uint64_t quantity;
    uint32_t id;
    uint8_t  side;
    Instrument* prev;   // intrusive list links (self-loop when inactive)
    Instrument* next;
    uint32_t slot;      // slot index back-pointer for DensePtrSet
};

inline constexpr uint32_t kNoSlot = 0xFFFFFFFFu;

inline std::vector<Instrument> make_universe(std::size_t n)
{
    std::vector<Instrument> uv;
    uv.reserve(n);

    for (std::size_t i = 0; i < n; ++i)
    {
        uv.push_back(Instrument{
            static_cast<uint64_t>(i * 2654435761u),
            100u + i,
            static_cast<uint32_t>(i),
            static_cast<uint8_t>(i & 1u),
            nullptr,
            nullptr,
            kNoSlot,
        });
        uv.back().prev = &uv.back();
        uv.back().next = &uv.back();
    }

    return uv;
}