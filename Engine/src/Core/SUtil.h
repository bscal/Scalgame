#pragma once

#include <stdint.h>

uint32_t DJBHash(const char* str, uint32_t length)
{
    uint32_t hash = 5381;
    uint32_t i = 0;
    for (i = 0; i < length; ++str, ++i)
    {
        hash = ((hash << 5) + hash) + (*str);
    }
    return hash;
}

template<typename T>
uint64_t Hash(const T* object)
{
    return DJBHash((const char*)object, sizeof(T));
}

struct TestHashStruct
{
    int x;
    int y;
    int z;
};

inline void TestHashes()
{
    TestHashStruct s = { 5, 1, 5 };
    auto hash = Hash(&s);

    TestHashStruct ss = { 19191, 112412, 50000 };
    auto hash2 = Hash(&ss);

    TestHashStruct sss = { 5, -1, 5 };
    auto hash3 = Hash(&sss);
}
