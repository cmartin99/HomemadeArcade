#pragma once
#include <stdlib.h>
#include <chrono>
#include "vulkan.h"
#include "TdEngine.h"
#include "TdRandom.h"
#include "DataTypes.h"

namespace NewGame {

extern AppMemory* memory;
static char temp_text[1000];

template<typename T>
ALWAYS_INLINE void memclear(T* p) { memset(p, 0, sizeof(T)); }
template<typename T>
ALWAYS_INLINE void memclear(T* p, int n) { memset(p, 0, sizeof(T) * n); }
ALWAYS_INLINE AppState* GetAppState() { return (AppState*)memory->perm_ram; }
ALWAYS_INLINE bool IsAppInitialized() { return memory != nullptr; }

}
