#pragma once

#include <stdint.h>

struct uint128
{
    uint64_t i0;
    uint64_t i1;
};

constexpr uint128 MurmurHash3_x64_128(
    uint64_t seed, const char* const key, uint64_t len);