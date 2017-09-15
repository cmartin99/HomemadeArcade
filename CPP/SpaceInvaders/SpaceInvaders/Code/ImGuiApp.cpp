
const char* gui_text[] = {
	nullptr,
	"New Game",
	"Resume Game",
	"Exit Game"
};

struct Gui
{
	enum Item
	{
		None,
		NewGame,
		ResumeGame,
		ExitGame
	};
};

const char* GuiText(GuiID item) { return gui_text[item]; }

void ImGuiUpdateMainMenu(Player* player)
{
	auto game_state = GetGameState();
	ImGui *gui = &player->gui;
	TdSpriteBatch* sprite_batch = gui->sprite_batch;

	int bord = 2;
	TdRect rect = {0, 0, 0, 40};
	TdRect main_rect = tdRectCenter(player->viewport, 400, 3 * (rect.h + 1) + bord * 2 - 1);
	tdVkDrawBox(sprite_batch, main_rect, Color(0), bord, Color(1));

	int hgap = 1;
	rect = {main_rect.x + bord, main_rect.y + bord, main_rect.w - bord * 2, rect.h};

	if (DoMenuItem(gui, Gui::NewGame, rect, hgap))
	{
		GameInstanceNew(player);
	}
	else if (DoMenuItem(gui, Gui::ResumeGame, rect, hgap))
	{
	}
	else if (DoMenuItem(gui, Gui::ExitGame, rect, hgap))
	{
		ExitGame();
	}
}

void ImGuiUpdateGamePlay(Player* player)
{
}

void ImGuiUpdate(Player* player)
{
	assert(player);
	TIMED_BLOCK(ImGuiUpdate);
	player->gui.new_hot = Gui::None;

	if (player->mode == Player::pm_Menu)
	{
		ImGuiUpdateMainMenu(player);
	}
	else if (player->mode == Player::pm_Play)
	{
		ImGuiUpdateGamePlay(player);
	}

	player->gui.hot = player->gui.new_hot;
}