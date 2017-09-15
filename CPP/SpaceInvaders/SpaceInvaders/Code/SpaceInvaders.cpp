
#include <stdlib.h>
#include <chrono>
#include "vulkan.h"
#include "TdEngine.h"

using namespace glm;
using namespace eng;

#include "TdRandom.h"
#include "TdPoint.h"
#include "TdJSonParser.h"
#include "DataTypes.h"

namespace SpaceInvaders {

#include "Common.cpp"
#include "GameInstance.cpp"
#include "GameInput.cpp"
#include "GameUpdate.cpp"
#include "GameRender.cpp"
#include "ImGuiApp.cpp"

void RunOneFrame(double delta_secs)
{
	if (IsGameInitialized())
	{
		auto game_state = GetGameState();
		eng::total_seconds += delta_secs;
		game_state->seconds = delta_secs;
		game_state->total_seconds += delta_secs;
		game_state->scratch_arena.used = 0;

		HandleInput();
		UpdateFrame();
		RenderFrame();
	}
}

}
