
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

void RenderDebugTimedBlocks(const Gamer* gamer, TdArray<TdTimedBlockCounter>& timed_blocks, int* y)
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
		tdVkDrawBox(sprite_batch, 0, *y, gamer->viewport.width, debug_text_height - 1, debug_bar_col);
		tdVkDrawText(sprite_batch, temp_text, len, 2, *y, i % 2 == 0 ? Colors::Gray : Colors::White, debug_text_depth, debug_text_scale);
		*y += debug_text_height;
	}
}
#endif

void RenderDebug(const Gamer* gamer)
{
	auto app_state = GetAppState();
	TdVkInstance* vulkan = app_state->renderer.vulkan;
	TdSpriteBatch* sprite_batch = app_state->renderer.sprite_batch;
	DebugData* debug_data = &app_state->debug_data;
	TdInputState* input = &app_state->input;
	Sim* sim = &app_state->sim;
	int y = last_debug_y + 4;

	int len = sprintf(temp_text, "fps: %4d | mem (bytes): eng %s (%.1f), perm %s (%.1f), main %s (%.1f), scratch %s (%.1f), total %d | draws: %s",
			(int)(1 / app_state->seconds),
			sprintf_comma(eng_arena.used, commas1), (float)eng_arena.used / eng_arena.size * 100.0,
			sprintf_comma(app_state->perm_arena.used, commas2), (float)app_state->perm_arena.used / app_state->perm_arena.size * 100.0,
			sprintf_comma(app_state->main_arena.used, commas3), (float)app_state->main_arena.used / app_state->main_arena.size * 100.0,
			sprintf_comma(app_state->scratch_arena.used, commas4), (float)app_state->scratch_arena.used / app_state->scratch_arena.size * 100.0,
			eng_arena.used + app_state->perm_arena.used + app_state->main_arena.used + app_state->scratch_arena.used,
			sprintf_comma(debug_data->draw_primitive_count, commas5)
			);

	tdVkDrawBox(sprite_batch, 0, y, gamer->viewport.width, debug_text_height - 1, debug_bar_col);
	tdVkDrawText(sprite_batch, temp_text, len, 2, y, Colors::White, debug_text_depth, debug_text_scale);
	y += debug_text_height;

	if (app_state->debug_data.debug_verbosity > 1)
		RenderDebugTimedBlocks(gamer, timed_blocks, &y);

	last_debug_y = y;
}

void RenderSim(Sim* sim, Gamer* gamer, const TdRect& rect)
{
	TIMED_BLOCK(RenderSim);
	assert(sim);
	assert(gamer);
	auto app_state = GetAppState();
	float elapsed = sim->seconds;
	TdSpriteBatch* sprite_batch = app_state->renderer.sprite_batch;
}

VkResult RenderGame()
{
	TIMED_BLOCK(RenderGame);
	auto app_state = GetAppState();
	assert(app_state->renderer.vulkan);
	auto vulkan = app_state->renderer.vulkan;

	VkResult err;
	Gamer* gamer = app_state->gamers.ptr;

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
	render_pass_begin_info.renderArea = gamer->scissor_rect;
	render_pass_begin_info.clearValueCount = 2;
	render_pass_begin_info.pClearValues = clear_values;
	render_pass_begin_info.framebuffer = vulkan->frames[current_frame].frame_buffer;

	vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(command_buffer, 0, 1, &gamer->viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &gamer->scissor_rect);

	{
	TIMED_BLOCK(RenderFrame);
	app_state->debug_data.draw_primitive_count = 0;

	if (app_state->sim.is_active)
	{
		switch (gamer->screen_mode)
		{			
			case sm_Gameplay:
				RenderSim(&app_state->sim, gamer, GetViewportRect(gamer));
				break;
		}
	}
	}

	ImGuiUpdate(gamer);
	last_debug_y = 0;
	if (app_state->debug_data.debug_verbosity > 0) 
	{
		RenderDebug(gamer);
	}

	tdVkSpriteBatchPresent(app_state->renderer.sprite_batch, command_buffer);
	tdVkSpriteBatchPresent(app_state->renderer.gui_sprite_batch, command_buffer);

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

