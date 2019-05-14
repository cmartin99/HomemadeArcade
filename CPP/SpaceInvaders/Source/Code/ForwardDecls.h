#pragma once
#include "Common.h"

namespace NewGame {

void SimNew();
void SimEnd();
void UpdateSim(Sim*);
Player* PlayerNew(Sim*, Gamer* = nullptr, Player* = nullptr);
void InitApplication(AppMemory&, eng::TdVkInstance*, char*, uint16);
void CloseApplication();
void HandleMouseWheel(int16);
void InputClearMouseHandlers();
VkResult RenderGame();

}