#pragma once

#include "Core.h"
#include "SHash.hpp"
#include "SMemory.h"

#include <assert.h>
#include <stdint.h>

inline float ClampF(float min, float max, float value)
{
    return fmaxf(min, fminf(max, value));
}

inline Rectangle RectangleExpand(const Rectangle& rect, float width, float height)
{
    Rectangle r;
    r.x = rect.x - width / 2.0f;
    r.y = rect.y - height / 2.0f;
    r.width = rect.width + width / 2.0f;
    r.height = rect.height + height / 2.0f;
    return r;
}

template<typename T>
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
