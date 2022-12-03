#pragma once

#include <stdint.h>

//#include "Murmur.h"

constexpr size_t FNV_offset_basis = 0xcbf29ce484222325;
constexpr size_t FNV_prime = 0x100000001b3;

[[nodiscard]] constexpr size_t SDBMHashMerge(
    size_t seed, const unsigned char* const str, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        seed = static_cast<size_t>(str[i]) + (seed << 6) + (seed << 16) - seed;
    }
    return seed;
}

[[nodiscard]] constexpr size_t FNVHash(
    const unsigned char* const str, size_t length)
{
    size_t val = FNV_offset_basis;
    for (size_t i = 0; i < length; ++i)
    {
        val ^= static_cast<size_t>(str[i]);
        val *= FNV_prime;
    }
    return val;
}

[[nodiscard]] constexpr size_t FNVHashMerge(
    size_t val, const unsigned char* const str, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        val ^= static_cast<size_t>(str[i]);
        val *= FNV_prime;
    }
    return val;
}

template<typename T>
[[nodiscard]] constexpr size_t SHashMerge(size_t seed, const T& object)
{
    return FNVHash(&reinterpret_cast<const unsigned char&>(object), sizeof(T));
}
