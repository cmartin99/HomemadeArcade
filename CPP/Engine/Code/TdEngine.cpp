#include "TdEngine.h"
#include "TdPool.cpp"
#include "TdPath.cpp"
#include "TdHashTable.cpp"
#include "TdHelpers.cpp"
#include "TdQuadTree.cpp"
#include "TdRandom.cpp"
#include "TdThread.cpp"
#include "TdEntity.cpp"
#include "TdSimplexNoise.cpp"
#include "TdVkInstance.cpp"
#include "TdVkTexture.cpp"
#include "TdSpriteBatch.cpp"
#include "TdLineRenderer.cpp"
#include "TdLineRenderer2D.cpp"
#include "TdRingRenderer.cpp"
#include "TdSphereRenderer.cpp"
#include "TdGui.cpp"
#include "TdGuiIm.cpp"
#include "TdJSonParser.cpp"
#include "TdPreprocessor.cpp"

namespace eng {

TdMemoryArena eng_arena;
char *shader_path = nullptr;
double total_seconds;

#ifdef _PROFILE_
TdArray<TdTimedBlockCounter> timed_blocks;
uint32 draw_calls;
uint32 sprite_draws;
uint32 sprite_batches;
#endif

}