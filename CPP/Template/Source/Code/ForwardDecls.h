#pragma once
#include "Common.h"

namespace NewGame {

void SimNew();
void SimEnd();
void UpdateSim(Sim*);
Player* PlayerNew(Sim*, Gamer* = nullptr);
void InitApplication(AppMemory&, eng::TdVkInstance*);
void CloseApplication();
void HandleMouseWheel(int16);
VkResult RenderGame();
const char* GuiText(TdImGuiContext);
const char* GuiTipText(TdImGuiContext);

}