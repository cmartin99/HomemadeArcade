
static float debug_text_scale = 0.5f;
static float debug_text_depth = 1.0f;
static int32 debug_text_height = 16;
static Color debug_bar_col(0,0,0,0.8);

#ifdef _PROFILE_

static TdTimedBlockCounter *sorted_timed_blocks[1000];

void SortTimedBlocks(int n)
{
	assert(n <= 1000);
	--n;
	for (int32 c = 0 ; c < n; ++c)
	{
		for (int32 d = 0 ; d < n - c; ++d)
		{
			if (sorted_timed_blocks[d]->total_cycles < sorted_timed_blocks[d + 1]->total_cycles)
			{
				auto swap = sorted_timed_blocks[d];
				sorted_timed_blocks[d] = sorted_timed_blocks[d + 1];
				sorted_timed_blocks[d + 1] = swap;
			}
		}
	}
}

void RenderDebugTimedBlocks(const Renderer* renderer, TdArray<TdTimedBlockCounter>& timed_blocks, int* y)
{
	auto app_state = GetAppState();
	TdSpriteBatch* sprite_batch = app_state->renderer.sprite_batch;

	for (int32 i = 0; i < timed_blocks.count; ++i) sorted_timed_blocks[i] = timed_blocks.ptr + i;
	SortTimedBlocks(timed_blocks.count);

	for (int32  i = 0; i < timed_blocks.count; i++)
	{
		TdTimedBlockCounter* p = sorted_timed_blocks[i];
		int len = 0;
		if (p->total_hits > 1)
		{
			len = sprintf(temp_text, "%s:  tot:%s, hits:%s | low:%s avg:%s, hi:%s, last:%s",
				p->name,
				sprintf_comma(p->total_cycles, commas1),
				sprintf_comma(p->total_hits, commas2),
				sprintf_comma(p->lowest_cycles, commas3),
				sprintf_comma(p->total_cycles / p->total_hits, commas4),
				sprintf_comma(p->highest_cycles, commas5),
				sprintf_comma(p->last_cycles, commas6)
				);
		}
		else
		{
			len = sprintf(temp_text, "%s: %s", p->name, sprintf_comma(p->total_cycles, commas1));
		}
		tdVkDrawBox(sprite_batch, 0, *y, renderer->viewport.width, debug_text_height - 1, debug_bar_col);
		tdVkDrawTextDF(sprite_batch, temp_text, len, 2, *y, i % 2 == 0 ? Colors::Gray : Colors::White, debug_text_depth, debug_text_scale);
		*y += debug_text_height;
	}
}
#endif

void RenderDebug(const Renderer* renderer)
{
	auto app_state = GetAppState();
	DebugData* debug_data = &app_state->debug_data;
	TdInputState* input = &app_state->input;
	TdVkInstance* vulkan = renderer->vulkan;
	TdSpriteBatch* sprite_batch = renderer->sprite_batch;

	int y = last_debug_y + 4;
	bool split = true;

	if (!split)
	{
		int len = sprintf(temp_text, "fps: %d | mem (bytes): eng %s (%.1f), perm %s (%.1f), main %s (%.1f), scratch %s (%.1f), total %d | draws: %s",
				(int)(1 / app_state->seconds),
				sprintf_comma(eng_arena.used, commas8), (float)eng_arena.used / eng_arena.size * 100.0,
				sprintf_comma(app_state->perm_arena.used, commas9), (float)app_state->perm_arena.used / app_state->perm_arena.size * 100.0,
				sprintf_comma(app_state->main_arena.used, commas10), (float)app_state->main_arena.used / app_state->main_arena.size * 100.0,
				sprintf_comma(app_state->scratch_arena.used, commas11), (float)app_state->scratch_arena.used / app_state->scratch_arena.size * 100.0,
				eng_arena.used + app_state->perm_arena.used + app_state->main_arena.used + app_state->scratch_arena.used,
				sprintf_comma(debug_data->draw_primitive_count, commas12)
				);

		tdVkDrawBox(sprite_batch, 0, y, renderer->viewport.width, debug_text_height - 1, debug_bar_col);
		tdVkDrawTextDF(sprite_batch, temp_text, len, 2, y, Colors::White, debug_text_depth, debug_text_scale);
		y += debug_text_height;
	}
	else
	{
		int len = sprintf(temp_text, "fps: %d | mem (bytes): eng %s (%.1f), perm %s (%.1f), main %s (%.1f), scratch %s (%.1f), total %d | draws: %s",
				(int)(1 / app_state->seconds),
				sprintf_comma(eng_arena.used, commas8), (float)eng_arena.used / eng_arena.size * 100.0,
				sprintf_comma(app_state->perm_arena.used, commas9), (float)app_state->perm_arena.used / app_state->perm_arena.size * 100.0,
				sprintf_comma(app_state->main_arena.used, commas10), (float)app_state->main_arena.used / app_state->main_arena.size * 100.0,
				sprintf_comma(app_state->scratch_arena.used, commas11), (float)app_state->scratch_arena.used / app_state->scratch_arena.size * 100.0,
				eng_arena.used + app_state->perm_arena.used + app_state->main_arena.used + app_state->scratch_arena.used,
				sprintf_comma(debug_data->draw_primitive_count, commas12)
				);

		tdVkDrawBox(sprite_batch, 0, y, renderer->viewport.width, debug_text_height - 1, debug_bar_col);
		tdVkDrawTextDF(sprite_batch, temp_text, len, 2, y, Colors::White, debug_text_depth, debug_text_scale);
		y += debug_text_height;

/*		len = sprintf(temp_text, "mouse : %d, %d, %.1f, %.1f",
				app_state->input.mouse.mouse_pos.x,
				app_state->input.mouse.mouse_pos.y,
				gamer->player ? gamer->player->mouse_pos.x : 0.f,
				gamer->player ? gamer->player->mouse_pos.y : 0.f
				);

		tdVkDrawBox(sprite_batch, 0, y, gamer->viewport.width, debug_text_height - 1, debug_bar_col);
		tdVkDrawTextDF(sprite_batch, temp_text, len, 2, y, Colors::White, debug_text_depth, debug_text_scale);
		y += debug_text_height;*/

		len = sprintf(temp_text, "net | connections: %d | bytes sent per sec: %d | bytes recv per sec: %d",
				0,
				app_state->bytes_sent_last_second,
				app_state->bytes_recv_last_second
				);

		// len = sprintf(temp_text, "net | sent: frame: %d, second: %d | recv: frame: %d, second: %d",
		// 		app_state->bytes_sent_per_frame,
		// 		app_state->bytes_sent_last_second,
		// 		app_state->bytes_recv_per_frame,
		// 		app_state->bytes_recv_last_second
		// 		);

		tdVkDrawBox(sprite_batch, 0, y, renderer->viewport.width, debug_text_height - 1, debug_bar_col);
		tdVkDrawTextDF(sprite_batch, temp_text, len, 2, y, Colors::White, debug_text_depth, debug_text_scale);
		y += debug_text_height;

	}

	if (app_state->debug_data.debug_verbosity > 1)
		RenderDebugTimedBlocks(renderer, timed_blocks, &y);

	last_debug_y = y;
}

void RenderLogErrors(Renderer* renderer)
{/*
	auto app_state = GetAppState();
	TdSpriteBatch* sprite_batch = renderer->sprite_batch;

	int x = 3, y = last_debug_y + 4;
	for (int i = app_state->curr_log_error - 1; i >= 0; --i)
	{
		char *s = app_state->log_error_text + i * app_state->max_log_error_text_len;
		if (s[0] != '\0')
		{
			tdVkDrawTextDF(sprite_batch, s, 0, x, y, Colors::White, 1.f, 0.6f);
			y += 18;
		}
	}

	for (int i = app_state->max_log_errors - 1; i >= app_state->curr_log_error; --i)
	{
		char *s = app_state->log_error_text + i * app_state->max_log_error_text_len;
		if (s[0] != '\0')
		{
			tdVkDrawTextDF(sprite_batch, s, 0, x, y, Colors::White, 1.f, 0.6f);
			y += 18;
		}
	}

	last_debug_y = y;	*/
}

void RenderServer(Renderer* renderer)
{
	TIMED_BLOCK(RenderServer);
	auto app_state = GetAppState();
	TdSpriteBatch* sprite_batch = renderer->sprite_batch;
	TdRect rect = { 0, renderer->log_offset_y, (int32)renderer->viewport.width, (int32)renderer->viewport.height};
	float text_depth = 1.f, text_scale = 0.6f;

	int connection_count = 0;
	int gamer_count = 0, gamer_enabled_count = 0;
	int map_area = 0, map_radius = 0, player_ratio = 0, agar_ratio = 0, virus_ratio = 0, spawner_ratio = 0;
	int player_count = 0, player_enabled_count = 0;
	int player_cell_count = 0, player_enabled_cell_count = 0;
	int shot_count = 0, shot_enabled_count = 0, shot_cap = 0;
	int total_mass = 0;

	Sim* sim = &app_state->sim;
	if (sim->is_active)
	{
		map_radius = sim->world_size / 2;
		map_area = (int)(g_PI * (map_radius * map_radius));
		connection_count = sim->connections.count;
		gamer_count = sim->gamers.count;
		for (int j = 0; j < sim->gamers.count; ++j) {
			if (sim->gamers[j].gamer_id > 0) ++gamer_enabled_count;
		}
		player_count = sim->players.count;
		player_enabled_count = sim->player_count;
		player_ratio = sim->player_count ? map_area / sim->player_count : 0;
		agar_ratio = map_area / sim->agar.cap;
		virus_ratio = map_area / sim->viruses.cap;
		spawner_ratio = map_area / sim->spawners.cap;
		player_cell_count = sim->player_cells.count;
		for (int j = 0; j < sim->player_cells.count; ++j)
		{
			if (sim->player_cells[j].enabled)
			{
				++player_enabled_cell_count;
				total_mass += (int)sim->player_cells[j].mass_target;
			}
		}
		shot_count = sim->shots.count;
		shot_cap = sim->shots.cap;
		for (int j = 0; j < sim->shots.count; ++j) {
			if (sim->shots[j].enabled) ++shot_enabled_count;
		}
	}

	sprintf_comma(total_mass, temp_text2);

	int len = sprintf(temp_text, "Sim Active: %s\nSim Paused: %s\nMap Radius/Area: %d/%d\nConnections: %d\nGamers: %d/%d\nPlayers: %d/%d\nCells: %d/%d\nShots: %d/%d/%d\nTotal Mass: %s\nPlayer ratio: %d\nAgar ratio: %d\nVirus ratio: %d\nSpawner ratio: %d\nLargest Packet Size: %d\nHighest Agar Changed Count: %d\nHighest Shots Changed Count: %d",
		sim ? sim->is_active ? "Yes" : "No" : "No Sim", 
		sim ? sim->is_paused ? "Yes" : "No" : "", 
		map_radius, map_area,
		connection_count, 
		gamer_enabled_count, gamer_count, 
		player_enabled_count, player_count,
		player_enabled_cell_count, player_cell_count,
		shot_enabled_count, shot_count, shot_cap,
		temp_text2,
		player_ratio,
		agar_ratio,
		virus_ratio,
		spawner_ratio,
		app_state->packet_size_largest,
		sim->agar_changed_largest_count,
		sim->shots_changed_largest_count
		);

	tdVkDrawTextDF(sprite_batch, temp_text, len, rect.x + 4, rect.y + 4, Colors::White, text_depth, text_scale);
}

VkResult RenderGame()
{
	auto app_state = GetAppState();
	Renderer* renderer = &app_state->renderer;
	auto vulkan = renderer->vulkan;
	assert(vulkan);

	VkResult err;

	err = vulkan->fpAcquireNextImageKHR(vulkan->device, vulkan->swap_chain, UINT64_MAX, vulkan->semaphore_present_complete, (VkFence)NULL, &vulkan->current_frame);
	if (err)
	{
		tdDisplayError("fpAcquireNextImageKHR", err);
		return err;
	}

	//tdVkWaitForFrameFence(vulkan);
	tdVkUpdateFreeResources(vulkan);
	uint32 current_frame = vulkan->current_frame;

	VkCommandBuffer command_buffer = vulkan->frames[current_frame].draw_command_buffer;
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

	VkRenderPassBeginInfo render_pass_begin_info = {};
	render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_begin_info.renderPass = vulkan->render_pass;
	render_pass_begin_info.renderArea = renderer->scissor_rect;
	render_pass_begin_info.clearValueCount = 2;
	render_pass_begin_info.pClearValues = clear_values;
	render_pass_begin_info.framebuffer = vulkan->frames[current_frame].frame_buffer;

	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(command_buffer, 0, 1, &renderer->viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &renderer->scissor_rect);

	{
	// Render Frame

	app_state->debug_data.draw_primitive_count = 0;
	RenderServer(renderer);
	ImGuiUpdate(renderer);
	last_debug_y = 0;
	if (app_state->debug_data.debug_verbosity > 0) RenderDebug(renderer);
	RenderLogErrors(renderer);
	}

	tdVkSpriteBatchPresent(renderer->sprite_batch, command_buffer);
	tdVkSpriteBatchPresent(renderer->gui_sprite_batch, command_buffer);

	vkCmdEndRenderPass(command_buffer);

	VkImageMemoryBarrier pre_present_barrier = {};
	pre_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	pre_present_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	pre_present_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	pre_present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	pre_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	pre_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	pre_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	pre_present_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	pre_present_barrier.image = vulkan->frames[current_frame].image;

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
	submit_info.pWaitSemaphores = &vulkan->semaphore_present_complete;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &vulkan->semaphore_render_complete;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	err = vkQueueWaitIdle(vulkan->queue);
	if (err)
	{
		tdDisplayError("vkQueueWaitIdle", err);
		return err;
	}

	tdVkUpdateFreeCommandBuffers(vulkan, vulkan->queue);

	err = vkQueueSubmit(vulkan->queue, 1, &submit_info, NULL);//vulkan.frames[current_frame].fence);
	if (err)
	{
		tdDisplayError("vkQueueSubmit", err);
		return err;
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &vulkan->swap_chain;
	present_info.pImageIndices = &current_frame;
	present_info.pWaitSemaphores = &vulkan->semaphore_render_complete;
	present_info.waitSemaphoreCount = 1;

	err = vulkan->fpQueuePresentKHR(vulkan->queue, &present_info);
	if (err)
	{
		tdDisplayError("queuePresent", err);
		return err;
	}

	return VK_SUCCESS;
}
