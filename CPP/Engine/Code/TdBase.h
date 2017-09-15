#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <xstring>
#include <windows.h>
#include <winuser.h>
#include <assert.h>
#include "Xinput.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "glm/gtc/constants.hpp"
#include "glm/glm.hpp"
#include "Vulkan.h"

#define ALWAYS_INLINE __forceinline
#define NEVER_INLINE __declspec(noinline)
#define DEFAULT_FENCE_TIMEOUT 100000000000
#define countof(X) (sizeof(X) / sizeof((X)[0]))

using glm::int8;
using glm::int16;
using glm::int32;
using glm::int64;
using glm::uint8;
using glm::uint16;
using glm::uint32;
using glm::uint64;
using glm::quat;

using glm::distance;
using glm::length;
using glm::lookAt;
using glm::normalize;

typedef glm::mat4 Matrix;
typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::vec4 Color;

namespace eng {

#ifdef _PROFILE_
#define TIMED_BLOCK(name) TdTimedBlock _timed_block_##name(#name);
#define TIMED_BLOCK_ONCE(name) TdTimedBlock _timed_block_##name(#name, true);
extern uint32 draw_calls;
extern uint32 sprite_draws;
extern uint32 sprite_batches;
#else
#define TIMED_BLOCK(name)
#define TIMED_BLOCK_ONCE(name)
#endif

extern char *shader_path;
static const Vector3 up3(0.0f, -1.0f, 0.0f);
extern double total_seconds;

struct TdMemoryArena
{
	uint8 *base;
	uint64 size;
	uint64 used;
};

extern TdMemoryArena eng_arena;

ALWAYS_INLINE void tdMemoryArenaInit(TdMemoryArena& arena, uint64 size, void* base_address)
{
	arena.size = size;
	arena.base = (uint8*)base_address;
	arena.used = 0;
}

ALWAYS_INLINE void* tdMalloc(TdMemoryArena& arena, uint64 size)
{
	assert(arena.size - arena.used >= size);
	void *result = arena.base + arena.used;
	arena.used += size;
	return result;
}

template<typename T>
ALWAYS_INLINE T* tdMalloc(TdMemoryArena& arena)
{
	return (T*)tdMalloc(arena, sizeof(T));
}

template<typename T>
ALWAYS_INLINE T* tdMalloc(TdMemoryArena& arena, uint64 count)
{
	return (T*)tdMalloc(arena, sizeof(T) * count);
}

}
