
static uint32 my_draw_calls, my_sprite_draws, my_sprite_batches = 0;
void ImGuiUpdate(Player* player);

void RenderDebug(Player* player, TdSpriteBatch* sprite_batch, TdPoint2 world_pos)
{
	auto game_state = GetGameState();
	TdVkInstance& vulkan = *game_state->vulkan;
	auto debug_data = &game_state->debug_data;
	TdInputState* input = &game_state->input;

	int y = world_pos.y + 1;
	int x = world_pos.x;
	tdVkDrawBox(sprite_batch, x, y, player->viewport.width, game_state->debug_data.last_debug_height, Color(0, 0, 0, 0.35), 1, Colors::White);

	// -------------------------------------------------------------------

	int len = sprintf(temp_text, "fps: %d | draws: %d, %d, %d",
		(int)(1 / game_state->seconds),
		my_draw_calls, my_sprite_batches, my_sprite_draws
	);

	tdVkDrawTextDF(sprite_batch, temp_text, len, x + 2, y, Colors::White, 0.6f);
	y += 16;

	// -------------------------------------------------------------------

	len = sprintf(temp_text, "mem: eng: %s (%.1f), perm: %s (%.1f), main: %s (%.1f), scratch: %s (%.1f), total used: %s, total alloc: %s",
			sprintf_comma(eng_arena.used, commas8), (float)eng_arena.used / eng_arena.size * 100.0,
			sprintf_comma(game_state->perm_arena.used, commas9), (float)game_state->perm_arena.used / game_state->perm_arena.size * 100.0,
			sprintf_comma(game_state->main_arena.used, commas10), (float)game_state->main_arena.used / game_state->main_arena.size * 100.0,
			sprintf_comma(game_state->scratch_arena.used, commas11), (float)game_state->scratch_arena.used / game_state->scratch_arena.size * 100.0,
			sprintf_comma(eng_arena.used + game_state->perm_arena.used + game_state->main_arena.used + game_state->scratch_arena.used, commas12),
			sprintf_comma(eng_arena.size + game_state->perm_arena.size + game_state->main_arena.size + game_state->scratch_arena.size, commas7)
		);

	//tdVkDrawBox(sprite_batch, x, y, player->viewport.width, 15, Colors::Black * Color(0, 0, 0, 0.65));
	tdVkDrawTextDF(sprite_batch, temp_text, len, x + 2, y, Colors::White, 0.6f);
	y += 16;

	// -------------------------------------------------------------------
#ifdef _PROFILE_
	if (debug_data->debug_verbosity >= 3)
	{
		TdTimedBlockCounter* blocks = timed_blocks.ptr;
		for (uint64 i = 0; i < timed_blocks.count; i++)
		{
			TdTimedBlockCounter* p = blocks + i;
			len = 0;
			if (p->hits > 1)
			{
				len = sprintf(temp_text, "%s:  tot:%s, hits:%s | low:%s avg:%s, hi:%s, last:%s",
					p->name,
					sprintf_comma(p->total_cycles, commas1),
					sprintf_comma(p->hits, commas2),
					sprintf_comma(p->lowest_cycles, commas3),
					sprintf_comma(p->total_cycles / p->hits, commas4),
					sprintf_comma(p->highest_cycles, commas5),
					sprintf_comma(p->last_cycles, commas6)
					);
			}
			else
			{
				len = sprintf(temp_text, "%s: %s", p->name, sprintf_comma(p->total_cycles, commas1));
			}
			//tdVkDrawBox(sprite_batch, x, y, GetPlayer(game_state, 1)->viewport.width, 15, Colors::Black * Color(0,0,0,0.65));
			tdVkDrawTextDF(sprite_batch, temp_text, len, x + 2, y, Colors::White, 0.5f);
			y += 16;
		}
	}
#endif
	game_state->debug_data.last_debug_height = y;
}

void RenderGame(Player* player)
{
	auto game_state = GetGameState();
	TdSpriteBatch *sprite_batch = game_state->sprite_batch;
	GameInstance *instance = game_state->instance;

	instance->invader_fleet_extent.x = FLT_MAX;
	instance->invader_fleet_extent.y = -FLT_MAX;
	TdRect rect = {0, 0, GameConsts::invader_size.x, GameConsts::invader_size.y};
	Invader *invader = instance->invader_fleet;
	Invader *invader_end = invader + instance->invader_count;

	while (invader < invader_end)
	{
		if (invader->index >= 0)
		{
			uint32 row = invader->index % GameConsts::wave_size.x;
			uint32 col = invader->index / GameConsts::wave_size.x;
			rect.x = instance->invader_fleet_pos.x + row * GameConsts::invader_spacing.x;
			rect.y = instance->invader_fleet_pos.y + col * GameConsts::invader_spacing.y;
			tdVkDrawBox(sprite_batch, rect, Color(1));

			if (rect.x < instance->invader_fleet_extent.x)
				instance->invader_fleet_extent.x = rect.x;
			if (rect.x + rect.w > instance->invader_fleet_extent.y)
				instance->invader_fleet_extent.y = rect.x + rect.w;
		}
		++invader;
	}

	rect.x = instance->ship->pos.x;
	rect.y = instance->ship->pos.y;
	rect.w = GameConsts::defender_size.x;
	rect.h = GameConsts::defender_size.y;
	tdVkDrawBox(sprite_batch, rect, Color(1));

	rect.w = GameConsts::bullet_size.x;
	rect.h = GameConsts::bullet_size.y;
	Bullet *bullet = instance->bullets;
	Bullet *bullet_end = bullet + instance->bullet_count;
	while (bullet < bullet_end)
	{
		if (bullet->alive)
		{
			rect.x = bullet->pos.x;
			rect.y = bullet->pos.y;
			tdVkDrawBox(sprite_batch, rect, Color(1));
		}
		++bullet;
	}

	rect.x = 40;
	rect.w = GameConsts::defender_size.x / 2;
	rect.h = GameConsts::defender_size.y / 2;
	rect.y = player->viewport.height - rect.h - 10;
	for (uint32 i = 0; i < player->lives; ++i)
	{
		tdVkDrawBox(sprite_batch, rect, Color(1));
		rect.x += rect.w + 10;
	}

	tdVkDrawTextDF(sprite_batch, "Score: 0", 0, 15, 10, Color(1), 1);
	tdVkDrawTextDF(sprite_batch, "Wave: 1", 0, 750, 10, Color(1), 1);
	tdVkDrawTextDF(sprite_batch, "Highscore: 0", 0, player->viewport.width - 300, 10, Color(1), 1);
}

VkResult RenderFrame(bool diagnostics_only = false)
{
	TIMED_BLOCK(RenderFrame);
	auto game_state = GetGameState();

	assert(game_state->vulkan);
	assert(game_state->sprite_batch);
	auto& vulkan = *game_state->vulkan;

	VkResult err;

	err = vulkan.fpAcquireNextImageKHR(vulkan.device, vulkan.swap_chain, UINT64_MAX, vulkan.semaphore_present_complete, (VkFence)NULL, &vulkan.current_frame);
	if (err)
	{
		tdDisplayError("fpAcquireNextImageKHR", err);
		return err;
	}

	//tdVkWaitForFrameFence(vulkan);
	tdVkUpdateFreeResources(vulkan);
	uint32 current_frame = vulkan.current_frame;

	VkCommandBuffer command_buffer = vulkan.frames[current_frame].draw_command_buffer;
	err = vkResetCommandBuffer(command_buffer, VK_FLAGS_NONE);
	if (err)
	{
		tdDisplayError("vkBeginCommandBuffer", err);
		return err;
	}

	VkCommandBufferBeginInfo command_buffer_begin_info = {};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	err = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
	if (err)
	{
		tdDisplayError("vkBeginCommandBuffer", err);
		return err;
	}

	VkClearColorValue default_clear_color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	VkClearValue clear_values[2];
	clear_values[0].color = default_clear_color;
	clear_values[1].depthStencil = { 1.0f, 0 };

	VkRect2D scissor_rect = {};
	scissor_rect.extent.width = vulkan.surface_width;
	scissor_rect.extent.height = vulkan.surface_height;

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = vulkan.render_pass;
	render_pass_begin_info.renderArea = scissor_rect;
	render_pass_begin_info.clearValueCount = 2;
	render_pass_begin_info.pClearValues = clear_values;
	render_pass_begin_info.framebuffer = vulkan.frames[current_frame].frame_buffer;
	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	//////////////// Draw Player   ///////////////////////////

	Player *player = game_state->player;
	vkCmdSetViewport(command_buffer, 0, 1, &player->viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &player->scissor_rect);

	//////////////// Draw Player   ///////////////////////////

	my_draw_calls = draw_calls;
	my_sprite_draws = sprite_draws;
	my_sprite_batches = sprite_batches;
	draw_calls = sprite_draws = sprite_batches = 0;

	if (!diagnostics_only || player->mode <= Player::pm_Menu)
	{
		switch (player->mode)
		{
			case Player::pm_Play:
				RenderGame(player);
				break;
		}
	}

	tdVkSpriteBatchPresent(*game_state->sprite_batch, command_buffer);

	ImGuiUpdate(player);

	if (game_state->debug_data.debug_verbosity)
		RenderDebug(player, game_state->gui_sprite_batch, TdPoint2(0, 0));

	tdVkDrawBox(game_state->gui_sprite_batch, 0, 0, game_state->vulkan->surface_width, game_state->vulkan->surface_height, Color(0), 1, Color(1));
	tdVkSpriteBatchPresent(*game_state->gui_sprite_batch, command_buffer);

	vkCmdEndRenderPass(command_buffer);

	//////////////////////////////////////////////////////////////////////

	VkImageMemoryBarrier pre_present_barrier = {};
	pre_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	pre_present_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	pre_present_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	pre_present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	pre_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	pre_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	pre_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	pre_present_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	pre_present_barrier.image = vulkan.frames[current_frame].image;

	VkImageMemoryBarrier* memory_barrier = &pre_present_barrier;
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_FLAGS_NONE, 0, NULL, 0, NULL, 1, &pre_present_barrier);

	err = vkEndCommandBuffer(command_buffer);
	if (err)
	{
		tdDisplayError("vkEndCommandBuffer", err);
		return err;
	}

	VkPipelineStageFlags submit_pipeline_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask = &submit_pipeline_stages;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &vulkan.semaphore_present_complete;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &vulkan.semaphore_render_complete;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	err = vkQueueWaitIdle(vulkan.queue);
	if (err)
	{
		tdDisplayError("vkQueueWaitIdle", err);
		return err;
	}

	tdVkUpdateFreeCommandBuffers(vulkan, vulkan.queue);

	err = vkQueueSubmit(vulkan.queue, 1, &submit_info, NULL);//vulkan.frames[current_frame].fence);
	if (err)
	{
		tdDisplayError("vkQueueSubmit", err);
		return err;
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &vulkan.swap_chain;
	present_info.pImageIndices = &current_frame;
	present_info.pWaitSemaphores = &vulkan.semaphore_render_complete;
	present_info.waitSemaphoreCount = 1;

	err = vulkan.fpQueuePresentKHR(vulkan.queue, &present_info);
	if (err)
	{
		tdDisplayError("queuePresent", err);
		return err;
	}

	return VK_SUCCESS;
}
