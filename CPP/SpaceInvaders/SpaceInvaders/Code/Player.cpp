
void PlayerLocalNew(Player* player)
{
	assert(player);
	//assert(player_count > 0 && player_count < 17);
	//assert(player_id >= 0 && player_id < player_count);

	auto game_state = GetGameState();
	memclear<Player>(player);
	player->mode = Player::pm_Menu;

	TdRect vp = GetViewportSize(0, 1);//player_id, player_count);
	player->viewport = {};
	player->viewport.x = vp.x;
	player->viewport.y = vp.y;
	player->viewport.width = vp.w;
	player->viewport.height = vp.h;
	player->viewport.minDepth = (float)0.0f;
	player->viewport.maxDepth = (float)1.0f;

	player->scissor_rect = {};
	player->scissor_rect.extent.width = player->viewport.width;
	player->scissor_rect.extent.height = player->viewport.height;
	player->scissor_rect.offset.x = player->viewport.x;
	player->scissor_rect.offset.y = player->viewport.y;

	player->gui.input = &game_state->input;
	player->gui.sprite_batch = game_state->gui_sprite_batch;
	player->gui.GuiText = &GuiText;
}
