#include "TdGui.h"

namespace eng {

// static Color mouse_default_color(1.0f, 0.8f, 0.1f, 1.0f);

// void tdGuiSetMouseColor(TdGui& gui, Color color)
// {
// 	gui.mouse_color = color;
// }

// void tdGuiSetMouseColor(TdGui& gui)
// {
// 	tdGuiSetMouseColor(gui, mouse_default_color);
// }

// void tdGuiSetMousePos(TdGui& gui, float x, float y)
// {
// 	if (x < 0) x = 0; else if (x >= gui.viewport.w) x = gui.viewport.w - 1;
// 	if (y < 0) y = 0; else if (y >= gui.viewport.h) y = gui.viewport.h - 1;
// 	gui.mouse_pos.x = x;
// 	gui.mouse_pos.y = y;
// }

// void tdGuiSetMousePosHIO(TdGui& gui, float x, float y)
// {
// 	gui.is_mouse_visible = (x >= 0 && x < gui.viewport.w && y >= 0 && y < gui.viewport.h);
// 	tdGuiSetMousePos(gui, x, y);
// }

// void tdGuiCenterMouse(TdGui& gui)
// {
// 	tdGuiSetMousePos(gui, gui.viewport.w / 2, gui.viewport.h / 2);
// }

TdRect tdGetWindowRect(const TdWindow& win)
{
	TdRect rect = { win.pos.x - win.border_size, win.pos.y - win.border_size, win.size.x + win.border_size + win.border_size, win.size.y + win.border_size + win.border_size };
	return rect;
}

TdRect tdGetClientRect(const TdWindow& win)
{
	TdRect rect = { win.pos.x, win.pos.y, win.size.x, win.size.y };
	return rect;
}

ALWAYS_INLINE bool tdWinIsEnabled(TdWindow* win)
{
	return win->check_is_enabled
		? win->check_is_enabled(win)
		: (win->flags & TD_WINFLAG_Enabled) > 0;
}

ALWAYS_INLINE bool tdWinIsEnabledVisible(TdWindow* win)
{
	return win->check_is_enabled
		? (win->flags & TD_WINFLAG_Visible) > 0 && win->check_is_enabled(win)
		: (win->flags & TD_WINFLAG_EnabledVisible) == TD_WINFLAG_EnabledVisible;
}

void tdWinAlwaysTop(TdGui& gui, TdWindow& win, bool always_top)
{
	if (!always_top)
		tdWinClearFlags(win, TD_WINFLAG_AlwaysTop);
	else
	{
		tdWinAddFlags(win, TD_WINFLAG_AlwaysTop);
		TdWindow *p_win = &win;
		if (win.parent)
		{
			if ((*win.parent->children)[win.parent->children->GetCount() - 1] != p_win)
			{
				win.parent->children->Remove(p_win);
				win.parent->children->Add(p_win);
			}
		}
		else
		{
			assert(gui.windows.count);
			if (gui.windows[gui.windows.count - 1] != p_win)
			{
				tdArrayRemoveRef(gui.windows, p_win);
				tdArrayAdd(gui.windows, p_win);
			}
		}
	}
}

void tdAddWindow(TdGui& gui, TdWindow* parent, TdWindow& win)
{
	win.parent = parent;
	win.flags |= TD_WINFLAG_EnabledVisible;

	if (parent == nullptr)
	{
		tdArrayAdd(gui.windows, &win);
	}
	else
	{
		if (!parent->children) parent->children = new TdArray1<TdWindow*>(4);
		parent->children->Add(&win);
	}
}

void tdFreeWindow(TdGui& gui, TdWindow& win)
{
	if (win.parent)
	{
		win.parent->children->Remove(&win);
		if (!win.parent->children->GetCount())
		{
			delete win.parent->children;
			win.parent->children = nullptr;
		}
		win.parent = nullptr;
	}
	else
	{
		tdArrayRemoveRef(gui.windows, &win);
	}
}

TdPoint2 tdWinWorldPos(const TdWindow *win)
{
	assert(win);
	TdPoint2 world_pos = win->pos;
	TdWindow *parent = win->parent;
	while (parent)
	{
		world_pos.x += parent->pos.x;
		world_pos.y += parent->pos.y;
		parent = parent->parent;
	}
	return world_pos;
}

// bool tdGuiHandleInput(TdGui& gui, const TdInputState& input)
// {
// 	gui.input = input;
// 	if (gui.is_mouse_active)
// 	{
// 		float mx = input.gamepad.thumb_left.x;
// 		float my = input.gamepad.thumb_left.y;
// 		if (mx == 0 && my == 0)
// 		{
// 			gui.mouse_vel = 0.0f;
// 		}
// 		else
// 		{
// 			if (gui.mouse_vel == 0.0f) gui.mouse_vel = gui.mouse_speed * 0.15f;
// 			gui.mouse_vel = min<float>(gui.mouse_speed, gui.mouse_vel + gui.mouse_speed * input.seconds * 2.0f);

// 			Vector2 pos = gui.mouse_pos;
// 			pos.x += mx * gui.mouse_vel * input.seconds;
// 			pos.y -= my * gui.mouse_vel * input.seconds;
// 			tdGuiSetMousePos(gui, pos);
// 		}

// 		if (gui.mouse_edge_scroll)
// 		{
// 			float x = gui.mouse_pos.x == 0.0f ? -1.0f : gui.mouse_pos.x == gui.viewport.w - 1 ? 1.0f : 0.0f;
// 			float y = gui.mouse_pos.y == 0.0f ? 1.0f : gui.mouse_pos.y == gui.viewport.h - 1 ? -1.0f : 0.0f;
// 			if (x != 0 || y != 0) gui.mouse_edge_scroll(x, y, gui.mouse_edge_scroll_state);
// 		}
// 	}

// 	return false;
// }

void tdGuiCalcHovered(TdGui& gui, TdWindow& win, TdPoint2 world_pos, TdPoint2 mouse_pos)
{
	TdPoint2 win_pos;
	win_pos.x = world_pos.x + win.pos.x;
	win_pos.y = world_pos.y + win.pos.y;

	if (mouse_pos.x >= win_pos.x && mouse_pos.x < win_pos.x + win.size.x &&
		mouse_pos.y >= win_pos.y && mouse_pos.y < win_pos.y + win.size.y)
	{
		gui.win_hovered = &win;

		if (win.children)
		{
			TdWindow **win_ptr = win.children->ptr();
			uint64 count = win.children->GetCount();

			for (uint64 i = 0; i < count; ++i)
			{
				if (tdWinIsEnabledVisible(win_ptr[i]))
					tdGuiCalcHovered(gui, *win_ptr[i], win_pos, mouse_pos);
			}
		}
	}
}

void tdGuiUpdate(TdGui& gui, TdInputState& input)
{
	gui.mouse_pos_for_present = input.mouse.mouse_pos;
	gui.is_left_mouse_button_down = input.mouse.mb_left.button_ended_down;
	TdWindow *last_hovered = gui.win_hovered;
	gui.win_hovered = nullptr;

	TdPoint2 world_pos(0, 0);
	TdWindow** win_ptr = gui.windows.ptr;
	uint64 count = gui.windows.count;

	for (uint64 i = 0; i < count; ++i)
	{
		if (tdWinIsEnabledVisible(win_ptr[i]))
			tdGuiCalcHovered(gui, *win_ptr[i], world_pos, input.mouse.mouse_pos);
	}

	if (gui.win_hovered != last_hovered)
	{
		if (last_hovered && last_hovered->hover_end_handler)
			last_hovered->hover_end_handler(*last_hovered, TdPoint2());
		if (gui.win_hovered && gui.win_hovered->hover_start_handler)
			gui.win_hovered->hover_start_handler(*gui.win_hovered, input.mouse.mouse_pos - gui.win_hovered_world_pos);
		if (!gui.win_hovered) SetCursor(gui.cursors[0]);
		gui.tooltip_timer = total_seconds + 1.5;
	}

	if (gui.win_hovered)
	{
		if (gui.win_hovered->hover_handler)
			gui.win_hovered->hover_handler(*gui.win_hovered, input.mouse.mouse_pos - gui.win_hovered_world_pos);

		if (!gui.win_dragging)
		{
			if (gui.win_hovered->flags & TD_WINFLAG_Sizeable)
			{
				TdPoint2 mouse_pos = input.mouse.mouse_pos;
				TdPoint2 win_pos = gui.win_hovered_world_pos;
				int bs = 6;

				if (mouse_pos.x < win_pos.x + bs)
				{
					gui.win_border_drag_mode = TD_DRAGMODE_Left;
					SetCursor(gui.cursors[1]);
				}
				else if (mouse_pos.x > win_pos.x + gui.win_hovered->size.x - bs)
				{
					gui.win_border_drag_mode = TD_DRAGMODE_Right;
					SetCursor(gui.cursors[1]);
				}
				else if (mouse_pos.y < win_pos.y + bs)
				{
					gui.win_border_drag_mode = TD_DRAGMODE_Top;
					SetCursor(gui.cursors[2]);
				}
				else if (mouse_pos.y > win_pos.y + gui.win_hovered->size.y - bs)
				{
					gui.win_border_drag_mode = TD_DRAGMODE_Bottom;
					SetCursor(gui.cursors[2]);
				}
				else
				{
					gui.win_border_drag_mode = TD_DRAGMODE_None;
					SetCursor(gui.cursors[0]);
				}
			}

			if (input.mouse.mb_left.button_ended_down && (gui.win_hovered->flags & (TD_WINFLAG_Dragable | TD_WINFLAG_Sizeable)) > 0)
			{
				if (input.mouse.mb_left.half_transition_count)
				{
					gui.last_mouse_drag_pos = input.mouse.mouse_pos;
				}
				else if (input.mouse.mouse_pos != gui.last_mouse_drag_pos)
				{
					gui.win_dragging = gui.win_hovered;
					gui.last_mouse_drag_pos = input.mouse.mouse_pos;
					if (gui.win_dragging->drag_start_handler)
						gui.win_dragging->drag_start_handler(*gui.win_dragging, input.mouse.mouse_pos - gui.win_hovered_world_pos);
				}
			}
		}

		if (!gui.win_dragging && gui.win_hovered->click_handler)
		{
			int state_change_count = input.mouse.mb_left.half_transition_count;
			bool down = input.mouse.mb_left.button_ended_down;
			while (state_change_count--)
			{
				if (!down) gui.win_hovered->click_handler(*gui.win_hovered, input.mouse.mouse_pos - gui.win_hovered_world_pos);
				down = !down;
			}

			state_change_count = input.mouse.mb_right.half_transition_count;
			down = input.mouse.mb_right.button_ended_down;
			while (state_change_count--)
			{
				if (!down) gui.win_hovered->click_handler(*gui.win_hovered, input.mouse.mouse_pos - gui.win_hovered_world_pos);
				down = !down;
			}
		}
	}

	if (gui.win_dragging)
	{
		if (!input.mouse.mb_left.button_ended_down)
		{
			if (gui.win_dragging->drag_end_handler)
				gui.win_dragging->drag_end_handler(*gui.win_dragging, input.mouse.mouse_pos - gui.win_hovered_world_pos);
			gui.win_dragging = nullptr;
			SetCursor(gui.cursors[0]);
		}
		else
		{
			switch (gui.win_border_drag_mode)
			{
				case TD_DRAGMODE_None:
					gui.win_dragging->pos += input.mouse.mouse_pos - gui.last_mouse_drag_pos;
					if (gui.win_dragging->drag_handler)
						gui.win_dragging->drag_handler(*gui.win_dragging, input.mouse.mouse_pos - gui.win_hovered_world_pos);
					break;
				case TD_DRAGMODE_Left:
					gui.win_dragging->pos.x += input.mouse.mouse_pos.x - gui.last_mouse_drag_pos.x;
					gui.win_dragging->size.x -= input.mouse.mouse_pos.x - gui.last_mouse_drag_pos.x;
					break;
				case TD_DRAGMODE_Top:
					gui.win_dragging->pos.y += input.mouse.mouse_pos.y - gui.last_mouse_drag_pos.y;
					gui.win_dragging->size.y -= input.mouse.mouse_pos.y - gui.last_mouse_drag_pos.y;
					break;
				case TD_DRAGMODE_Right:
					gui.win_dragging->size.x += input.mouse.mouse_pos.x - gui.last_mouse_drag_pos.x;
					break;
				case TD_DRAGMODE_Bottom:
					gui.win_dragging->size.y += input.mouse.mouse_pos.y - gui.last_mouse_drag_pos.y;
					break;
			}

			gui.last_mouse_drag_pos = input.mouse.mouse_pos;
		}
	}
}

void tdGuiPresentWinCore(TdGui& gui, TdWindow* win, TdPoint2 world_pos)
{
	assert(win);
	if (!(win->flags & TD_WINFLAG_Visible)) return;

	world_pos.x += win->pos.x;
	world_pos.y += win->pos.y;

	if (win == gui.win_hovered)
	{
		gui.win_hovered_world_pos.x = world_pos.x;
		gui.win_hovered_world_pos.y = world_pos.y;
		//int w = gui.is_left_mouse_button_down ? 2 : 1;
		//tdVkDrawBox(gui.sprite_batch, world_pos.x - win.border_size - w, world_pos.y - win.border_size - w, win.size.x + win.border_size * 2 + w * 2, win.size.y + win.border_size * 2 + w * 2, Colors::None, w, Colors::White);
	}

	if (win->back_color.a > 0 || (win->border_size > 0 && win->border_color.a > 0) || win->texture == nullptr || (win->texture_rect != nullptr && (win->texture_rect->x != 0 || win->texture_rect->y != 0 || win->texture_rect->w < win->size.x || win->texture_rect->h < win->size.y)))
	{
		tdVkDrawBox(gui.sprite_batch, world_pos.x - win->border_size, world_pos.y - win->border_size, win->size.x + win->border_size * 2, win->size.y + win->border_size * 2, win->back_color, win->border_size, win->border_color);
	}

	if (win->tex_color.a > 0 && win->texture)
	{
		TdRect rect;
		if (win->texture_rect)
		{
			rect.x = win->texture_rect->x;
			rect.y = win->texture_rect->y;
			rect.w = win->texture_rect->w;
			rect.h = win->texture_rect->h;
		}
		else
		{
			rect.x = 0;
			rect.y = 0;
			rect.w = win->size.x;
			rect.h = win->size.y;
		}
		tdVkDrawBox(gui.sprite_batch, world_pos.x + rect.x, world_pos.y + rect.y, rect.w, rect.h, win->tex_color, *win->texture, win->texture_src_rect);
	}

	switch (win->type)
	{
		case TD_WINTYPE_TextBox:
		{
			TdTextBox* tb = (TdTextBox*)win;
			if (tb->text && tb->text_color.a > 0)
			{
				Color color(tb->text_color);
				if (!tdWinIsEnabled(tb)) color *= 0.3;

				if (tb->text_flags & TD_WINTEXT_Center)
				{
					Vector2 text_size;
					tdVkSpriteBatchGetTextSize(&gui.sprite_batch, tb->text, 0, text_size);
					tdVkDrawTextDF(gui.sprite_batch, tb->text, 0, world_pos.x + tb->text_pos.x + (tb->size.x - text_size.x * tb->text_scale) / 2, world_pos.y + tb->text_pos.y, color, tb->text_scale);
				}
				else if (tb->text_flags & TD_WINTEXT_Right)
				{
					Vector2 text_size;
					tdVkSpriteBatchGetTextSize(&gui.sprite_batch, tb->text, 0, text_size);
					tdVkDrawTextDF(gui.sprite_batch, tb->text, 0, world_pos.x - tb->text_pos.x + tb->size.x - text_size.x * tb->text_scale, world_pos.y + tb->text_pos.y, color, tb->text_scale);
				}
				else
				{
					tdVkDrawTextDF(gui.sprite_batch, tb->text, 0, world_pos.x + tb->text_pos.x, world_pos.y + tb->text_pos.y, color, tb->text_scale);
				}
			}
		} break;
	}

	if (win->draw)
		win->draw(win, &gui.sprite_batch, world_pos);

	if (win->children)
	{
		uint64 child_count = win->children->GetCount();
		TdWindow** child_ptr = win->children->ptr();

		for (uint64 i = 0; i < child_count; ++i)
		{
			if (*child_ptr != gui.win_dragging)
				tdGuiPresentWinCore(gui, *child_ptr, world_pos);
			child_ptr++;
		}
	}
}

void tdDrawToolTip(TdGui& gui)
{
	TdTextBox *tooltip = gui.win_hovered->tooltip;
	tooltip->flags |= TD_WINFLAG_EnabledVisible;
	tooltip->text = gui.win_hovered->tooltip_text;
	Vector2 text_size;
	tdVkSpriteBatchGetTextSize(&gui.sprite_batch, tooltip->text, 0, text_size);
	tooltip->size.x = text_size.x * tooltip->text_scale + tooltip->text_pos.x * 2 + 2;
	tooltip->size.y = text_size.y * tooltip->text_scale + tooltip->text_pos.y * 2 + 4;
	TdPoint2 mpos = gui.mouse_pos_for_present;
	tooltip->pos.x = tdClamp(mpos.x - tooltip->size.x / 2, 0, gui.viewport.w - tooltip->size.x);
	tooltip->pos.y = tdClamp(mpos.y - 8 - tooltip->size.y, 0, gui.viewport.h - tooltip->size.y);
	tdGuiPresentWinCore(gui, tooltip, TdPoint2(0,0));
	tdWinClearFlags(tooltip, TD_WINFLAG_EnabledVisible);
}

void tdGuiPresent(TdGui& gui, VkCommandBuffer command_buffer)
{
	TdPoint2 world_pos(0, 0);
	uint64 count = gui.windows.count;
	TdWindow** win_ptr = gui.windows.ptr;

	for (uint64 i = 0; i < count; ++i)
	{
		tdGuiPresentWinCore(gui, *win_ptr++, world_pos);
	}

	if (gui.win_dragging)
	{
		tdGuiPresentWinCore(gui, gui.win_dragging, tdWinWorldPos(gui.win_dragging) - gui.win_dragging->pos);
	}
	else if (gui.win_hovered && gui.win_hovered->tooltip && gui.win_hovered->tooltip_text && total_seconds > gui.tooltip_timer)
	{
		tdDrawToolTip(gui);
	}

	tdVkSpriteBatchPresent(gui.sprite_batch, command_buffer);
}

void tdGuiInit(TdGui& gui, TdMemoryArena& mem, TdVkInstance& vulkan)
{
	gui.mem = &mem;
	gui.vulkan = &vulkan;
	tdArrayInit(gui.windows, 100);
	gui.viewport.x = 0;
	gui.viewport.y = 0;
	gui.viewport.w = vulkan.surface_width;
	gui.viewport.h = vulkan.surface_height;
	tdVkSpriteBatchInit(gui.sprite_batch, vulkan);

	gui.cursors[0] = LoadCursor(NULL, IDC_ARROW);
	gui.cursors[1] = LoadCursor(NULL, IDC_SIZEWE);
	gui.cursors[2] = LoadCursor(NULL, IDC_SIZENS);
	//gui.cursors[3] = LoadCursor(NULL, IDC_SIZEWE);
	//gui.cursors[4] = LoadCursor(NULL, IDC_SIZEWE);
}

}