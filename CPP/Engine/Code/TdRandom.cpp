/*
* PCG Random Number Generation for C.
*
* Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* For additional information about the PCG random number generation scheme,
* including its license and other licensing options, visit
*
*       http://www.pcg-random.org
*/

/*
* This code is derived from the full C implementation, which is in turn
* derived from the canonical C++ PCG implementation. The C++ version
* has many additional features and is preferable if you can use C++ in
* your project.
*/

#include "TdRandom.h"

namespace eng {

static TdPcg32Random pcg32_global = PCG32_INITIALIZER;

void tdRandomSeed(TdPcg32Random* rng, uint64 initstate, uint64 initseq)
{
	rng->state = 0U;
	rng->inc = (initseq << 1u) | 1u;
	tdRandomNext(rng);
	rng->state += initstate;
	tdRandomNext(rng);
}

void tdRandomSeed(uint64 seed, uint64 seq)
{
	tdRandomSeed(&pcg32_global, seed, seq);
}

int32 tdRandomNext(TdPcg32Random* rng)
{
	uint64 oldstate = rng->state;
	rng->state = oldstate * 6364136223846793005ULL + rng->inc;
	uint32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32 rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

int32 tdRandomNext()
{
	return tdRandomNext(&pcg32_global);
}

int32 tdRandomNext(TdPcg32Random* rng, int32 bound)
{
	// To avoid bias, we need to make the range of the RNG a multiple of
	// bound, which we do by dropping output less than a threshold.
	// A naive scheme to calculate the threshold would be to do
	//
	//     uint32 threshold = 0x100000000ull % bound;
	//
	// but 64-bit div/mod is slower than 32-bit div/mod (especially on
	// 32-bit platforms).  In essence, we do
	//
	//     uint32 threshold = (0x100000000ull-bound) % bound;
	//
	// because this version will calculate the same modulus, but the LHS
	// value is less than 2^32.

	int32 threshold = -bound % bound;

	// Uniformity guarantees that this loop will terminate.  In practice, it
	// should usually terminate quickly; on average (assuming all bounds are
	// equally likely), 82.25% of the time, we can expect it to require just
	// one iteration.  In the worst case, someone passes a bound of 2^31 + 1
	// (i.e., 2147483649), which invalidates almost 50% of the range.  In
	// practice, bounds are typically small and only a tiny amount of the range
	// is eliminated.
	for (;;)
	{
		int32 r = tdRandomNext(rng);
		if (r >= threshold)
		{
			int32 result = r % bound;
			return result;
		}
	}
}

//inline
double tdRandomNextDouble(TdPcg32Random* rng)
{
	return tdRandomNext(rng, INT_MAX) / (double)INT_MAX;
}

int32 tdRandomNext(int32 bound)
{
	return tdRandomNext(&pcg32_global, bound);
}

}