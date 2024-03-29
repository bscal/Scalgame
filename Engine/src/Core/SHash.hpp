#pragma once

#include "Core.h"

#include <MurmurHash3/Murmur.h>

constexpr global_var uint32_t FNV32_offset_basis = 0x811c9dc5;
constexpr global_var uint32_t FNV32_prime = 0x1000193;
          
constexpr global_var uint64_t FNV64_offset_basis = 0xcbf29ce484222325;
constexpr global_var uint64_t FNV64_prime = 0x100000001b3;

_FORCE_INLINE_ constexpr uint64_t
FNVHash64(const uint8_t* const str, size_t length) noexcept
{
    uint64_t val = FNV64_offset_basis;
    for (size_t i = 0; i < length; ++i)
    {
        val ^= str[i];
        val *= FNV64_prime;
    }
    return val;
}

_FORCE_INLINE_ constexpr uint32_t
FNVHash32(const uint8_t* const str, size_t length) noexcept
{
    uint32_t val = FNV32_offset_basis;
    for (size_t i = 0; i < length; ++i)
    {
        val ^= str[i];
        val *= FNV32_prime;
    }
    return val;
}
