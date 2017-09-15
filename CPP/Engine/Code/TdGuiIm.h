#pragma once
#include "TdDataTypes.h"

namespace eng {

typedef uint32 GuiID;

struct ImGui
{
	GuiID hot;
	GuiID new_hot;
	GuiID active;
	TdInputState *input;
	TdSpriteBatch *sprite_batch;
	const char* (*GuiText)(GuiID);
};

bool DoMenuItem(ImGui*, GuiID item, TdRect&, int hgap = 0, bool enabled = true);
bool DoButton(ImGui*, GuiID item, const TdRect&, bool enabled = true);

}