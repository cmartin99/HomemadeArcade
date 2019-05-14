#pragma once
#include "TdBase.h"

namespace eng {

#define PCG32_INITIALIZER   { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

typedef struct TdPcgStateSetSeq64
{
	uint64 state;
	uint64 inc;
} TdPcg32Random;

void tdRandomSeed(uint64 initstate, uint64 initseq);
void tdRandomSeed(TdPcg32Random* rng, uint64 initstate, uint64 initseq);
int32 tdRandomNext(void);
int32 tdRandomNext(TdPcg32Random* rng);
inline int32 tdRandomNext(TdPcg32Random& rng) { return  tdRandomNext(&rng); }
int32 tdRandomNext(int32 bound);
int32 tdRandomNext(TdPcg32Random* rng, int32 bound);
inline int32 tdRandomNext(TdPcg32Random& rng, int32 bound) { return tdRandomNext(&rng, bound); }
inline int32 tdRandomNext(TdPcg32Random* rng, int32 min_bound, int32 max_bound) { return tdRandomNext(rng, max_bound - min_bound + 1) + min_bound; }
inline int32 tdRandomNext(TdPcg32Random& rng, int32 min_bound, int32 max_bound) { return tdRandomNext(&rng, max_bound - min_bound + 1) + min_bound; }
inline double tdRandomNextDouble(TdPcg32Random* rng) { return tdRandomNext(rng, INT_MAX) / (double)INT_MAX; }
inline double tdRandomNextDouble(TdPcg32Random& rng) { return tdRandomNext(&rng, INT_MAX) / (double)INT_MAX; }

}
