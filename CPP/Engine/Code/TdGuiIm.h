#pragma once
#include "TdDataTypes.h"

namespace eng {

typedef uint32 GuiID;

struct TdImGuiContext
{
	GuiID id;
	void* data;
};

struct TdImGui
{
	GuiID hot;
	GuiID new_hot;
	GuiID active;
	TdInputState *input;
	TdSpriteBatch *sprite_batch;
	TdPoint2 text_offset;
	int32 button_border_size;
	float button_text_scale, menu_text_scale, tooltip_text_scale;
	Color button_back_color, button_border_color, button_disabled_back_color, button_hot_back_color, button_pressed_back_color;
	Color button_text_color, button_disabled_text_color, button_hot_text_color, button_pressed_text_color;
	Color menu_back_color, menu_border_color, menu_disabled_back_color, menu_hot_back_color, menu_pressed_back_color;
	Color menu_text_color, menu_disabled_text_color, menu_hot_text_color, menu_pressed_text_color;
	Color frame_back_color, frame_border_color;
	Color tooltip_back_color, tooltip_border_color, tooltip_text_color;
	TdRect hot_rect;
	const char* (*GuiText)(TdImGuiContext);
	const char* (*GuiTipText)(TdImGuiContext);
};

void RenderFrame(TdImGui *gui, const TdRect&);
void RenderMenuItem(TdImGui*, TdImGuiContext, const TdRect&, const char *text, bool enabled = true);
void RenderMenuItem(TdImGui*, TdImGuiContext, const TdRect&, const char *text, Color text_color, bool enabled = true);
void RenderMenuItem(TdImGui*, TdImGuiContext, const TdRect&, const char *text, Color back_color, Color text_color, bool enabled = true, bool text_centered = true);
bool DoMenuItem(TdImGui*, TdImGuiContext, const TdRect&, bool enabled = true);
bool DoMenuItem(TdImGui*, GuiID, const TdRect&, bool enabled = true);
bool DoButton(TdImGui*, TdImGuiContext, const TdRect&, bool enabled = true);
bool DoButton(TdImGui*, GuiID, const TdRect&, bool enabled = true);
void DoToolTip(TdImGui*, TdImGuiContext);
void DoToolTip(TdImGui*);
void tdImGuiInit(TdImGui*);

}