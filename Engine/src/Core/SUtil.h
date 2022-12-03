#pragma once

#include "SHash.hpp"
#include "SMemory.h"

#include <assert.h>
#include <stdint.h>

template<class T>
struct SAllocator
{
    typedef T value_type;

    SAllocator() = default;

    template<class U>
    constexpr SAllocator(const SAllocator <U>&) noexcept {}

    [[nodiscard]] T* allocate(size_t n)
    {
        assert(n < (SIZE_MAX / sizeof(T)));
        auto p = static_cast<T*>(Scal::MemAlloc(n * sizeof(T)));
        if (p)
        {
            report(p, n);
            return p;
        }
        else
        {
            S_LOG_ERR("SAllocator bad allocation!");
            assert(p);
            return nullptr;
        }
    }

    void deallocate(T* p, size_t n) noexcept
    {
        report(p, n, 0);
        Scal::MemFree(p);
    }
private:
    void report(T* p, size_t n, bool alloc = true) const
    {
        const char* str = (alloc) ? "Alloc" : "Dealloc";
        S_LOG_INFO("%sing %d bytes at %p", str, n, p);
    }
};

template<class T, class U>
bool operator==(const SAllocator <T>&, const SAllocator <U>&) { return true; }

template<class T, class U>
bool operator!=(const SAllocator <T>&, const SAllocator <U>&) { return false; }


// TODO hash test stuff remove

struct TestHashStruct
{
    int x;
    int y;
    int z;
};

inline void TestHashes()
{
    TestHashStruct s = { 5, 1, 5 };
    auto hash = SHashMerge(0, s);

    TestHashStruct ss = { 5, 2, 5 };
    auto hash2 = SHashMerge(0, ss);

    TestHashStruct sss = { 5, -1, 5 };
    auto hash3 = SHashMerge(0, sss);

    TestHashStruct s4 = { 1, 5, 1 };
    auto hash4 = SHashMerge(0, s4);

    TestHashStruct s5 = { 6, 1, 5 };
    auto hash5 = SHashMerge(0, s5);

    TestHashStruct s6 = { 4, 1, 5 };
    auto hash6 = SHashMerge(0, s6);

    TestHashStruct s7 = { 5, 1, 6 };
    auto hash7 = SHashMerge(0, s7);

    TestHashStruct s8 = { -5, 1, 5 };
    auto hash8 = SHashMerge(0, s8);

    TestHashStruct s9 = { 5, 1, 4 };
    auto hash9 = SHashMerge(0, s9);

    size_t mod = 8 * 2;
    auto hashM = hash % mod;
    auto hashM2 = hash2 % mod;
    auto hashM3 = hash3 % mod;
    auto hashM4 = hash4 % mod;
    auto hashM5 = hash5 % mod;
    auto hashM6 = hash6 % mod;
    auto hashM7 = hash7 % mod;
    auto hashM8 = hash8 % mod;
    auto hashM9 = hash9 % mod;
}
