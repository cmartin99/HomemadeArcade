
namespace eng {

void RenderFrame(TdImGui* gui, const TdRect& rect)
{
	if (gui->frame_border_color.a)
		tdVkDrawBoxO(gui->sprite_batch, rect.x, rect.y, rect.w, rect.h, gui->frame_back_color, 1, gui->frame_border_color);
	else
		tdVkDrawBox(gui->sprite_batch, rect.x, rect.y, rect.w, rect.h, gui->frame_back_color);
}

void RenderButton(TdImGui* gui, TdImGuiContext context, const TdRect& rect, bool enabled)
{
	Color back_color = gui->button_back_color;
	Color text_color = gui->button_text_color;

	if (!enabled)
	{
		back_color = gui->button_disabled_back_color;
		text_color = gui->button_disabled_text_color;
	}
	else if (context.id == gui->active && context.id == gui->hot)
	{
		back_color = gui->button_pressed_back_color;
		text_color = gui->button_pressed_text_color;
	}
	else if (context.id == gui->active || (context.id == gui->hot && !gui->active))
	{
		back_color = gui->button_hot_back_color;
		text_color = gui->button_hot_text_color;
	}

	if (gui->button_border_color.a)
		tdVkDrawBoxO(gui->sprite_batch, rect.x, rect.y, rect.w, rect.h, back_color, gui->button_border_size, gui->button_border_color);
	else
		tdVkDrawBox(gui->sprite_batch, rect.x, rect.y, rect.w, rect.h, back_color);

	tdVkDrawTextCentered(gui->sprite_batch, gui->GuiText(context), rect.x + rect.w * 0.5f + gui->text_offset.x, rect.y + rect.h * 0.5f + gui->text_offset.y, text_color, 1, gui->button_text_scale);
}

void RenderMenuItem(TdImGui* gui, TdImGuiContext context, const TdRect& rect, const char* text, Color back_color, Color text_color, bool enabled, bool text_centered)
{
	tdVkDrawBox(gui->sprite_batch, rect.x, rect.y, rect.w, rect.h, back_color);

	if (text_centered)
		tdVkDrawTextCentered(gui->sprite_batch, text, rect.x + rect.w * 0.5f + gui->text_offset.x, rect.y + rect.h * 0.5f + gui->text_offset.y, text_color, 1, gui->menu_text_scale);
	else
	{
		Vector2 size = tdVkSpriteBatchGetTextSize(gui->sprite_batch, text, 0, gui->menu_text_scale);
		tdVkDrawText(gui->sprite_batch, text, 0, rect.x + gui->text_offset.x + 8, rect.y + gui->text_offset.y + (rect.h - size.y) * 0.5f, text_color, 1, gui->menu_text_scale);
	}
}

void RenderMenuItem(TdImGui* gui, TdImGuiContext context, const TdRect& rect, const char* text, Color text_color, bool enabled)
{
	Color back_color = gui->menu_back_color;

	if (!enabled)
	{
		back_color = gui->menu_disabled_back_color;
	}
	else if (context.id == gui->active && context.id == gui->hot)
	{
		back_color = gui->menu_pressed_back_color;
	}
	else if (context.id == gui->active || (context.id == gui->hot && !gui->active))
	{
		back_color = gui->menu_hot_back_color;
	}

	RenderMenuItem(gui, context, rect, text, back_color, text_color, enabled);
}

void RenderMenuItem(TdImGui* gui, TdImGuiContext context, const TdRect& rect, const char* text, bool enabled)
{
	Color back_color = gui->menu_back_color;
	Color text_color = gui->menu_text_color;

	if (!enabled)
	{
		back_color = gui->menu_disabled_back_color;
		text_color = gui->menu_disabled_text_color;
	}
	else if (context.id == gui->active && context.id == gui->hot)
	{
		back_color = gui->menu_pressed_back_color;
		text_color = gui->menu_pressed_text_color;
	}
	else if (context.id == gui->active || (context.id == gui->hot && !gui->active))
	{
		back_color = gui->menu_hot_back_color;
		text_color = gui->menu_hot_text_color;
	}

	RenderMenuItem(gui, context, rect, text, back_color, text_color, enabled);
}

void RenderMenuItem(TdImGui* gui, TdImGuiContext context, const TdRect& rect, bool enabled)
{
	RenderMenuItem(gui, context, rect, gui->GuiText(context), enabled);
}

bool DoClickable(TdImGui* gui, TdImGuiContext context, const TdRect& rect)
{
	bool result = false;

	if (context.id == gui->active)
	{
		if (tdMouseIsLeftClick(gui->input->mouse))
		{
			if (context.id == gui->hot) result = true;
			gui->active = 0;
		}
	}
	else if (context.id == gui->hot)
	{
		gui->hot_rect = rect;
		if (tdMouseIsLeftPress(gui->input->mouse))
			gui->active = context.id;
	}

	TdPoint2 mouse_pos = gui->input->mouse.mouse_pos;
	if (mouse_pos.x >= rect.x && mouse_pos.x < rect.x + rect.w &&
		mouse_pos.y >= rect.y && mouse_pos.y < rect.y + rect.h)
		gui->new_hot = context.id;

	return result;
}

bool DoMenuItem(TdImGui* gui, TdImGuiContext context, const TdRect& rect, bool enabled)
{
	assert(gui);
	bool clicked = enabled ? DoClickable(gui, context, rect) : false;
	RenderMenuItem(gui, context, rect, enabled);
	return clicked;
}

bool DoMenuItem(TdImGui* gui, GuiID id, const TdRect& rect, bool enabled)
{
	return DoMenuItem(gui, {id, nullptr}, rect, enabled);
}

bool DoButton(TdImGui* gui, TdImGuiContext context, const TdRect& rect, bool enabled)
{
	assert(gui);
	bool clicked = enabled ? DoClickable(gui, context, rect) : false;
	RenderButton(gui, context, rect, enabled);
	return clicked;
}

bool DoButton(TdImGui* gui, GuiID id, const TdRect& rect, bool enabled)
{ 
	return DoButton(gui, {id, nullptr}, rect, enabled); 
}

void DoToolTip(TdImGui* gui, TdImGuiContext context)
{
	if (gui->hot)
	{
		const char *text = gui->GuiTipText({gui->hot, context.data});
		if (text)
		{
			Vector2 size = tdVkSpriteBatchGetTextSize(gui->sprite_batch, text, 0, gui->tooltip_text_scale);
			TdRect rect = {gui->input->mouse.mouse_pos.x, (int)(gui->input->mouse.mouse_pos.y - size.y - 12), (int)(size.x + 8), (int)(size.y + 8)};
			tdVkDrawBoxO(gui->sprite_batch, rect.x, rect.y, rect.w, rect.h, gui->tooltip_back_color, 1, gui->tooltip_border_color);
			rect.x += 4;
			rect.y += 4;
			tdVkDrawText(gui->sprite_batch, text, 0, rect.x, rect.y, gui->tooltip_text_color, 1, gui->tooltip_text_scale);
		}
	}
}

void DoToolTip(TdImGui* gui)
{
	DoToolTip(gui, {});
}

void tdImGuiInit(TdImGui* gui)
{
	assert(gui);

	gui->tooltip_text_scale = 0.5f;
	gui->tooltip_back_color = Color(0.1f, 0.1f, 0.1f, 1.0f);
	gui->tooltip_border_color = Color(1);
	gui->tooltip_text_color = Color(0.75f, 0.75f, 0.75f, 1);

	gui->button_text_scale = 0.75f;
	gui->button_back_color = Color(0.1f, 0.1f, 0.1f, 0.8f);
	gui->button_disabled_back_color = Color(0.09f, 0.09f, 0.09f, 0.8f);
	gui->button_hot_back_color = Color(0.2f, 0.2f, 0.2f, 0.8f);
	gui->button_pressed_back_color = Color(0.3f, 0.3f, 0.3f, 0.8f);
	gui->button_text_color = Color(0.5f, 0.5f, 0.5f, 1);
	gui->button_disabled_text_color = Color(0.3f, 0.3f, 0.3f, 1);
	gui->button_hot_text_color = Color(0.75f, 0.75f, 0.75f, 1);
	gui->button_pressed_text_color = Color(1);
	gui->button_border_color = Color(1);

	gui->menu_text_scale = 0.9f;
	gui->menu_back_color = Color(0.1f, 0.1f, 0.1f, 0.8f);
	gui->menu_disabled_back_color = Color(0.09f, 0.09f, 0.09f, 0.8f);
	gui->menu_hot_back_color = Color(0.2f, 0.2f, 0.2f, 0.8f);
	gui->menu_pressed_back_color = Color(0.3f, 0.3f, 0.3f, 0.8f);
	gui->menu_text_color = Color(0.5f, 0.5f, 0.5f, 1);
	gui->menu_disabled_text_color = Color(0.3f, 0.3f, 0.3f, 1);
	gui->menu_hot_text_color = Color(0.75f, 0.75f, 0.75f, 1);
	gui->menu_pressed_text_color = Color(1);
}

}