
struct Gui
{
	enum Item
	{
		None,
		NewGame,
		Exit,
		Debug,
		ExitToMainMenu,		
	};
};

const char* gui_text[] = {
	nullptr,
	"New Game",
	"Exit",
	"D",
	"X",
};

const char* gui_tip_text[] = {
	nullptr, //None,
	nullptr, //NewGame,
	nullptr, //Exit,
	nullptr, //Debug,
	nullptr, //ExitToMainMenu,
};

const char* GuiText(TdImGuiContext context) 
{
	return gui_text[context.id]; 
}

const char* GuiTipText(TdImGuiContext context)
{
	return gui_tip_text[context.id]; 
}

bool IsGuiInteracted(const Gamer* gamer)
{
	assert(gamer);
	return gamer->gui_interacted || gamer->gui->hot != Gui::None;
}

bool ImGuiUpdateTitleMenu(Gamer* gamer)
{
	auto app_state = GetAppState();
	TdImGui *gui = gamer->gui;
	TdSpriteBatch* sprite_batch = gui->sprite_batch;

	gui->menu_text_scale = 0.9f;
	int item_count = 2;
	int bord = 2;
	TdRect rect = {0, 0, 0, 50};
	TdRect main_rect = tdRectCenter(gamer->viewport, 400, item_count * (rect.h + 1) + bord * 2 - 1);
	tdVkDrawBoxO(sprite_batch, main_rect, Colors::Black * 0.9f, bord, Colors::White);

	int ygap = 1;
	rect = {main_rect.x + bord, main_rect.y + bord, main_rect.w - bord * 2, rect.h};

	if (DoMenuItem(gui, Gui::NewGame, rect))
	{
		SimNew();
		gamer->screen_mode = sm_Gameplay;		
		return true;
	}
	rect.y += rect.h + ygap;
	if (DoMenuItem(gui, Gui::Exit, rect))
	{
		SimEnd();
		CloseApplication();
		return true;
	}

	return false;
}

bool ImGuiGameplay(Gamer* gamer)
{
	auto app_state = GetAppState();
	TdImGui *gui = gamer->gui;

	gui->button_text_scale = 0.6f;
	int item_count = 2;
	TdRect rect;
	rect.w = 24;
	rect.y = 4;
	rect.x = gamer->viewport.width - rect.w * item_count - item_count * 4;
	rect.h = 24;

	if (DoButton(gui, Gui::Debug, rect))
	{
		if (++app_state->debug_data.debug_verbosity > 2)
			app_state->debug_data.debug_verbosity = 0;
		return true;
	}
	rect.x += rect.w + 4;
	if (DoButton(gui, Gui::ExitToMainMenu, rect))
	{
		gamer->screen_mode = sm_TitleMenu;
		return true;
	}

	return false;
}

void ImGuiUpdate(Gamer* gamer)
{
	assert(gamer);
	TIMED_BLOCK(ImGuiUpdate);

	gamer->gui_interacted = false;
	gamer->gui->new_hot = Gui::None;

	switch (gamer->screen_mode)
	{
		case sm_Gameplay:
			gamer->gui_interacted = ImGuiGameplay(gamer);
			break;

		case sm_TitleMenu:
			gamer->gui_interacted = ImGuiUpdateTitleMenu(gamer);
			break;
	}

	gamer->gui->hot = gamer->gui->new_hot;
	DoToolTip(gamer->gui);
}