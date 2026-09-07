#pragma once

#include "memory_pool_allocator.h"

#include <cstddef>
#include <utility>
#include <type_traits>

struct alignas(std::max_align_t) Block32 {
    std::byte data[32];
};

struct alignas(std::max_align_t) Block64 {
    std::byte data[64];
};

struct alignas(std::max_align_t) Block128 {
    std::byte data[128];
};

struct alignas(std::max_align_t) Block256 {
    std::byte data[256];
};

struct alignas(std::max_align_t) Block512 {
    std::byte data[512];
};

class SlabAllocator {
public:
    explicit SlabAllocator(std::size_t capacity_per_slab = 1024)
        : m_slab32(capacity_per_slab),
          m_slab64(capacity_per_slab),
          m_slab128(capacity_per_slab),
          m_slab256(capacity_per_slab),
          m_slab512(capacity_per_slab)
    {}

    template<typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        constexpr std::size_t size = sizeof(T);

        if constexpr (size <= 32) {
            auto* block = m_slab32.allocate();
            if (!block) return nullptr;

            return new (block) T(std::forward<Args>(args)...);
        }
        else if constexpr (size <= 64) {
            auto* block = m_slab64.allocate();
            if (!block) return nullptr;

            return new (block) T(std::forward<Args>(args)...);
        }
        else if constexpr (size <= 128) {
            auto* block = m_slab128.allocate();
            if (!block) return nullptr;

            return new (block) T(std::forward<Args>(args)...);
        }
        else if constexpr (size <= 256) {
            auto* block = m_slab256.allocate();
            if (!block) return nullptr;

            return new (block) T(std::forward<Args>(args)...);
        }
        else if constexpr (size <= 512) {
            auto* block = m_slab512.allocate();
            if (!block) return nullptr;

            return new (block) T(std::forward<Args>(args)...);
        }
        else {
            static_assert(sizeof(T) <= 512,
                "Object too large for slab allocator");
        }
    }

    template<typename T>
    void deallocate(T* ptr)
    {
        if (!ptr) return;

        ptr->~T();

        constexpr std::size_t size = sizeof(T);

        if constexpr (size <= 32) {
            m_slab32.deallocate(reinterpret_cast<Block32*>(ptr));
        }
        else if constexpr (size <= 64) {
            m_slab64.deallocate(reinterpret_cast<Block64*>(ptr));
        }
        else if constexpr (size <= 128) {
            m_slab128.deallocate(reinterpret_cast<Block128*>(ptr));
        }
        else if constexpr (size <= 256) {
            m_slab256.deallocate(reinterpret_cast<Block256*>(ptr));
        }
        else {
            m_slab512.deallocate(reinterpret_cast<Block512*>(ptr));
        }
    }

private:
    MemoryPool<Block32>  m_slab32;
    MemoryPool<Block64>  m_slab64;
    MemoryPool<Block128> m_slab128;
    MemoryPool<Block256> m_slab256;
    MemoryPool<Block512> m_slab512;
};
