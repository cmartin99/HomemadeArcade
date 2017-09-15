
namespace eng {

void RenderButton(ImGui* gui, GuiID item, const TdRect& rect, bool enabled)
{
	Color back_color(0.1f);
	Color text_color(0.5, 0.5, 0.5, 1);
	if (!enabled)
	{
		text_color = Color(0.3, 0.3, 0.3, 1);
	}
	else if (item == gui->active && item == gui->hot)
	{
		back_color = Color(0.3f);
		text_color = Color(1);
	}
	else if (item == gui->active || (item == gui->hot && !gui->active))
	{
		back_color = Color(0.2f);
		text_color = Color(0.75,0.75,0.75,1);
	}

	tdVkDrawBox(gui->sprite_batch, rect, back_color);
	tdVkDrawTextCenteredDF(gui->sprite_batch, gui->GuiText(item), rect, text_color, 0.8f);
}

void RenderMenuItem(ImGui* gui, GuiID item, const TdRect& rect, bool enabled)
{
	Color back_color(0.1f);
	Color text_color(0.5, 0.5, 0.5, 1);
	if (!enabled)
	{
		text_color = Color(0.3, 0.3, 0.3, 1);
	}
	else if (item == gui->active && item == gui->hot)
	{
		back_color = Color(0.3f);
		text_color = Color(1);
	}
	else if (item == gui->active || (item == gui->hot && !gui->active))
	{
		back_color = Color(0.2f);
		text_color = Color(0.75,0.75,0.75,1);
	}

	tdVkDrawBox(gui->sprite_batch, rect, back_color);
	tdVkDrawTextCenteredDF(gui->sprite_batch, gui->GuiText(item), rect, text_color, 0.8f);
}

bool DoClickable(ImGui* gui, GuiID item, const TdRect& rect)
{
	bool result = false;

	if (item == gui->active)
	{
		if (tdMouseIsLeftClick(gui->input->mouse))
		{
			if (item == gui->hot) result = true;
			gui->active = 0;
		}
	}
	else if (item == gui->hot)
	{
		if (tdMouseIsLeftPress(gui->input->mouse))
			gui->active = item;
	}

	TdPoint2 mouse_pos = gui->input->mouse.mouse_pos;
	if (mouse_pos.x >= rect.x && mouse_pos.x < rect.x + rect.w &&
		mouse_pos.y >= rect.y && mouse_pos.y < rect.y + rect.h)
		gui->new_hot = item;

	return result;
}

bool DoMenuItem(ImGui* gui, GuiID item, TdRect& rect, int hgap, bool enabled)
{
	assert(gui);
	bool clicked = enabled ? DoClickable(gui, item, rect) : false;
	RenderMenuItem(gui, item, rect, enabled);
	rect.y += rect.h + hgap;
	return clicked;
}

bool DoButton(ImGui* gui, GuiID item, const TdRect& rect, bool enabled)
{
	assert(gui);
	bool clicked = enabled ? DoClickable(gui, item, rect) : false;
	RenderButton(gui, item, rect, enabled);
	return clicked;
}

}