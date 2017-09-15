#pragma once
#include "TdDataTypes.h"

namespace eng {

enum TdWinType
{
	TD_WINTYPE_Window,
	TD_WINTYPE_TextBox
};

enum TdWinFlag
{
	TD_WINFLAG_Enabled = 0x01,
	TD_WINFLAG_Visible = 0x02,
	TD_WINFLAG_EnabledVisible = TD_WINFLAG_Enabled | TD_WINFLAG_Visible,
	TD_WINFLAG_Dragable = 0x04,
	TD_WINFLAG_Sizeable = 0x08,
	TD_WINFLAG_AlwaysTop = 0x10,
};

enum TdWinTextFlag
{
	TD_WINTEXT_Left = 0x01,
	TD_WINTEXT_Right = 0x02,
	TD_WINTEXT_Center = 0x04,
};

enum TdWinDragMode
{
	TD_DRAGMODE_None,
	TD_DRAGMODE_Left,
	TD_DRAGMODE_Top,
	TD_DRAGMODE_Right,
	TD_DRAGMODE_Bottom,
	TD_DRAGMODE_TopLeft,
	TD_DRAGMODE_TopRight,
	TD_DRAGMODE_BottomLeft,
	TD_DRAGMODE_BottomRight
};

struct TdWindow;
struct TdTextBox;

typedef void (*TdWindowDrawOverride)(TdWindow*, TdSpriteBatch*, TdPoint2 world_pos);
typedef void (*TdWindowEvent)(TdWindow&, TdPoint2 mouse_pos);
typedef bool (*TdWindowCheck)(TdWindow*);

struct TdWindow
{
	TdWindow* parent;
	uint16 flags;
	TdPoint2 pos;
	TdPoint2 size;
	TdWindowCheck check_is_enabled;
	TdArray1<TdWindow*>* children;
	Color back_color;
	Color border_color;
	int border_size;
	void* data;
	TdVkTexture* texture;
	TdRect* texture_rect;
	TdRect* texture_src_rect;
	Color tex_color;
	TdWindowDrawOverride draw;
	TdWinType type;
	TdTextBox* tooltip;
	char* tooltip_text;
	TdWindowEvent click_handler;
	TdWindowEvent hover_start_handler;
	TdWindowEvent hover_handler;
	TdWindowEvent hover_end_handler;
	TdWindowEvent drag_start_handler;
	TdWindowEvent drag_handler;
	TdWindowEvent drag_end_handler;
};

struct TdTextBox : public TdWindow
{
	TdPoint2 text_pos;
	Color text_color;
	char* text;
	float text_scale;
	uint16 text_flags;
};

struct TdGui
{
	TdMemoryArena* mem;
	TdVkInstance* vulkan;
	TdRect viewport;
	TdSpriteBatch sprite_batch;
	TdArray<TdWindow*> windows;
	TdWindow* win_hovered;
	TdPoint2 win_hovered_world_pos;
	TdWindow* win_dragging;
	TdWinDragMode win_border_drag_mode;
	TdPoint2 mouse_pos_for_present;
	TdPoint2 last_mouse_drag_pos;
	double tooltip_timer;
	bool is_left_mouse_button_down;
	HCURSOR cursors[5];
};

TdRect tdGetWindowRect(const TdWindow&);
TdRect tdGetClientRect(const TdWindow&);
void tdAddWindow(TdGui&, TdWindow* parent, TdWindow&);
void tdFreeWindow(TdGui&, TdWindow&);
ALWAYS_INLINE void tdWinSetFlags(TdWindow* win, uint16 flags) { win->flags = flags; }
ALWAYS_INLINE void tdWinAddFlags(TdWindow& win, uint16 flags) {	win.flags |= flags; }
ALWAYS_INLINE void tdWinAddFlags(TdWindow* win, uint16 flags) {	win->flags |= flags; }
ALWAYS_INLINE void tdWinClearFlags(TdWindow& win, uint16 flags = 0xffff) { win.flags &= ~flags; }
ALWAYS_INLINE void tdWinClearFlags(TdWindow* win, uint16 flags = 0xffff) { win->flags &= ~flags; }
ALWAYS_INLINE void tdWinSetTextFlags(TdTextBox& win, uint16 flags) { win.text_flags = flags; }
ALWAYS_INLINE void tdWinSetTextFlags(TdTextBox* win, uint16 flags) { win->text_flags = flags; }
ALWAYS_INLINE void tdWinEnabled(TdWindow* win, bool is_enabled = true) { if (is_enabled) tdWinAddFlags(win, TD_WINFLAG_Enabled); else tdWinClearFlags(win, TD_WINFLAG_Enabled); }
ALWAYS_INLINE bool tdWinIsEnabled(const TdWindow& win) { return (win.flags & TD_WINFLAG_Enabled) > 0; }
ALWAYS_INLINE bool tdWinIsEnabled(const TdWindow* win) { return (win->flags & TD_WINFLAG_Enabled) > 0; }
ALWAYS_INLINE void tdWinVisible(TdWindow* win, bool is_visible = true) { if (is_visible) tdWinAddFlags(win, TD_WINFLAG_Visible); else tdWinClearFlags(win, TD_WINFLAG_Visible); }
ALWAYS_INLINE bool tdWinIsVisible(const TdWindow& win) { return (win.flags & TD_WINFLAG_Visible) > 0; }
ALWAYS_INLINE bool tdWinIsVisible(const TdWindow* win) { return (win->flags & TD_WINFLAG_Visible) > 0; }
void tdWinAlwaysTop(TdGui & gui, TdWindow & win, bool always_top = true);
ALWAYS_INLINE void tdWinAlwaysTop(TdGui & gui, TdWindow * win, bool always_top = true) { assert(win); tdWinAlwaysTop(gui, *win, always_top); }
ALWAYS_INLINE TdWindow* tdWinGetChild(TdWindow& win, uint32 i)
{
	if (win.children)
	{
		return i >= 0 && i < win.children->GetCount() ? win.children->GetValue(i) : nullptr;
	}
	return nullptr;
}
ALWAYS_INLINE TdWindow* tdWinGetChild(TdWindow* win, uint32 i) { return tdWinGetChild(*win, i); }
TdPoint2 tdWinWorldPos(const TdWindow *);
// void tdGuiSetMouseColor(TdGui&);
// void tdGuiSetMouseColor(TdGui&, Color);
// void tdGuiSetMousePos(TdGui&, float x, float y);
// void tdGuiSetMousePosHIO(TdGui&, float x, float y);
// inline void tdGuiSetMousePos(TdGui& gui, Vector2 pos) { tdGuiSetMousePos(gui, pos.x, pos.y); }
// inline bool tdIsMouseMoved(TdGui& gui) { return gui.mouse_vel != 0.0f; }
// void tdGuiCenterMouse(TdGui&);
void tdGuiInit(TdGui &, TdMemoryArena &, TdVkInstance &);
void tdGuiUpdate(TdGui &, TdInputState &);
void tdGuiPresent(TdGui &, VkCommandBuffer);
}