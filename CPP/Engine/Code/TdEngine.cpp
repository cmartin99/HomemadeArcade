#include "TdEngine.h"
#include "TdHelpers.cpp"
#include "TdRandom.cpp"
#include "TdThread.cpp"
#include "TdVkInstance.cpp"
#include "TdVkTexture.cpp"
#include "TdSpriteBatch.cpp"
#include "TdGuiIm.cpp"

namespace eng {

TdMemoryArena eng_arena;
char *shader_path = nullptr;
double total_seconds;

#ifdef _PROFILE_
TdArray<TdTimedBlockCounter> timed_blocks;
#endif

}