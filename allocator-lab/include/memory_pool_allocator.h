#pragma once
/*
Memory pool allocator
- objects of same size
*/
#include <cstddef>
#include <new>
#include <utility>

template<typename T>
class MemoryPool{
private:
    union Node{
        Node* next;
        alignas(T) std::byte storage[sizeof(T)];
    };
public:
    explicit MemoryPool(std::size_t capacity): m_capacity(capacity) {
        if(capacity == 0)
            throw std::invalid_argument("capacity must be > 0");

        m_pool = new Node[capacity];

        for(std::size_t i=0;i<capacity-1;++i){
            m_pool[i].next = &m_pool[i+1];
        }

        m_pool[capacity-1].next = nullptr;
        m_freeList = &m_pool[0];
    }

    ~MemoryPool(){
        delete[] m_pool; // doesnt call destructors. so all objects must be trivial or destructed before hand
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    template<typename... Args>
    T* allocate(Args&&... args){
        if(!m_freeList) return nullptr;

        Node* node = m_freeList;
        m_freeList = m_freeList->next;

        return new (node->storage) T(std::forward<Args>(args)...);
    }

    void deallocate(T* ptr){
        if(!ptr) return;

        ptr->~T(); 

        Node* node = reinterpret_cast<Node*>(ptr);
        node->next = m_freeList;
        m_freeList = node;
    }

private:
    Node* m_pool{};
    Node* m_freeList{};

    std::size_t m_capacity{};
};
