#pragma once

#include "Core.h"

static_assert(sizeof(int64_t) == 8, "Only usable with 64 bit");

inline uint64_t SplitMixNext64(uint64_t val)
{
	uint64_t z = (val += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

internal constexpr float FloatFromBits(uint32_t i)
{
	return (i >> 8) * 0x1.0p-24f;
}

internal constexpr double DoubleFromBits(uint64_t i)
{
	return (i >> 11) * 0x1.0p-53;
}

internal constexpr uint64_t Rotl(const uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}

internal constexpr uint64_t Rotl32(const uint32_t x, int k)
{
	return (x << k) | (x >> (32 - k));
}

struct SRandom
{
	uint64_t Seed0;
	uint64_t Seed1;
	uint64_t Seed2;
	uint64_t Seed3;
};

void InitializeSRandom(SRandom* random, uint64_t seed)
{
	random->Seed0 = seed;
	random->Seed1 = SplitMixNext64(random->Seed0);
	random->Seed2 = SplitMixNext64(random->Seed1);
	random->Seed3 = SplitMixNext64(random->Seed2);
}

uint64_t Next(SRandom* randomState)
{
	const uint64_t result = Rotl(randomState->Seed1 * 5, 7) * 9;
	const uint64_t t = randomState->Seed1 << 17;

	randomState->Seed2 ^= randomState->Seed0;
	randomState->Seed3 ^= randomState->Seed1;
	randomState->Seed1 ^= randomState->Seed2;
	randomState->Seed0 ^= randomState->Seed3;

	randomState->Seed2 ^= t;
	randomState->Seed3 = Rotl(randomState->Seed3, 45);
	return result;
}

double NextDouble(SRandom* randomState)
{
	return DoubleFromBits(Next(randomState));
}

uint32_t NextInt(SRandom* randomState)
{
	return (uint32_t)Next(randomState);
}

float NextFloat(SRandom* randomState)
{
	return FloatFromBits(NextInt(randomState));
}

/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void Jump(SRandom* randomState)
{
	static const uint64_t JUMP[] =
	{
		0x180ec6d33cfd0aba, 
		0xd5a61266f0c9392c, 
		0xa9582618e03fc9aa, 
		0x39abdc4529b1661c
	};

	uint64_t s0 = 0;
	uint64_t s1 = 0;
	uint64_t s2 = 0;
	uint64_t s3 = 0;
	for (int i = 0; i < sizeof JUMP / sizeof * JUMP; i++)
	{
		for (int b = 0; b < 64; b++)
		{
			if (JUMP[i] & UINT64_C(1) << b)
			{
				s0 ^= randomState->Seed0;
				s1 ^= randomState->Seed1;
				s2 ^= randomState->Seed2;
				s3 ^= randomState->Seed3;
			}
			Next(randomState);
		}
	}
	randomState->Seed0 = s0;
	randomState->Seed1 = s1;
	randomState->Seed2 = s2;
	randomState->Seed3 = s3;
}



/* This is the long-jump function for the generator. It is equivalent to
   2^192 calls to next(); it can be used to generate 2^64 starting points,
   from each of which jump() will generate 2^64 non-overlapping
   subsequences for parallel distributed computations. */

void LongJump(SRandom* randomState)
{
	static const uint64_t LONG_JUMP[] = 
	{ 
		0x76e15d3efefdcbbf,
		0xc5004e441c522fb3,
		0x77710069854ee241,
		0x39109bb02acbe635 };

	uint64_t s0 = 0;
	uint64_t s1 = 0;
	uint64_t s2 = 0;
	uint64_t s3 = 0;
	for (int i = 0; i < sizeof LONG_JUMP / sizeof * LONG_JUMP; i++)
	{
		for (int b = 0; b < 64; b++) {
			if (LONG_JUMP[i] & UINT64_C(1) << b) {
				s0 ^= randomState->Seed0;
				s1 ^= randomState->Seed1;
				s2 ^= randomState->Seed2;
				s3 ^= randomState->Seed3;
			}
			Next(randomState);
		}
	}
	randomState->Seed0 = s0;
	randomState->Seed1 = s1;
	randomState->Seed2 = s2;
	randomState->Seed3 = s3;
}

// Middle-Square Weyl Sequence RNG
struct SRandomMiddleSquare
{
	uint64_t X;
	uint64_t W;
	uint64_t S;
};

uint64_t NextFastInt64(SRandomMiddleSquare* randomState)
{
	randomState->W += randomState->S;
	randomState->X = 
		((randomState->X * randomState->X + randomState->W) >> 32)
		| ((randomState->X * randomState->X + randomState->W) << 32);
	return randomState->X;
}

double NextFastDouble(SRandomMiddleSquare* randomState)
{
	double d = 1;
	while (d == 1)
	{
		d = double(NextFastInt64(randomState) / UINT64_MAX);
	}
	return d;
}
