#pragma once
/*
Arena allocator
- objects must have same lifetime
- objects must be trivial
*/
#include<cstddef>
#include<cstdint>
#include<new>
#include<utility>

class Arena{
public:
    Arena(std::size_t size) : m_size(size) {
        m_buffer = reinterpret_cast<std::byte*>(::operator new(size));
    }

    ~Arena(){
        ::operator delete(m_buffer);
        // note this just frees memory and doesnt call object destructors. so the objects need to be trivial
    }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    // move semantics are ignore as we are just benchmarking

    void* allocate(std::size_t size,std::size_t alignment = alignof(std::max_align_t)){
        uintptr_t current {reinterpret_cast<uintptr_t>(m_buffer + m_offset)};
        uintptr_t aligned { (current + alignment -1) & ~(alignment -1)};
        std::size_t padding {aligned - current};

        if(m_offset+padding+size > m_size){
            return nullptr;
        }

        m_offset += padding; // now we are on a valid mem location
        void* result{ m_buffer+m_offset}; 
        m_offset += size;
        return result;
    }

    template<typename T,typename... Args>
    T* construct(Args&&... args){
        void* object {allocate(sizeof(T),alignof(T))};
        if(object==nullptr) return nullptr;

        return new (object) T(std::forward<Args>(args)...);
    }

    void reset(){
        m_offset = 0;
    }

private:
    std::size_t m_size{};
    std::byte* m_buffer{};    
    std::size_t m_offset{};
};
