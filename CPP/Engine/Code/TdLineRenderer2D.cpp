#include "TdLineRenderer2D.h"

namespace eng {

uint16 AddLine(TdLineRenderer2D::LineStorage& storage, TdLineRenderer2D::LineData& line)
{
	line.id = (uint16)storage.lines.count + 1;
	tdArrayAdd(storage.lines, line);
	storage.is_dirty = true;
	return line.id;
}

uint16 tdVkAddLine(TdLineRenderer2D* renderer, uint32 flags, Vector2 start, Vector2 end, Color color)
{
	assert(renderer);
	TdLineRenderer2D::LineStorage& storage = (flags & TdLineRenderer2D::TD_ADDLINE_USAGE_HOST_BUFFER_BIT)
		? renderer->lines_cpu
		: renderer->lines_gpu;

	if (flags & TdLineRenderer2D::TD_ADDLINE_USAGE_BOX_OUTLINE_BIT)
	{
		tdVector2MinMax(start, end);
		TdLineRenderer2D::LineData line = { 0, Vector2(start.x, start.y), Vector2(end.x, start.y), color };
		uint16 id = AddLine(storage, line);
		line.start.x = end.x;
		AddLine(storage, line);
		line.end.x = start.x;
		line.start.y = end.y;
		AddLine(storage, line);
		line.start.x = start.x;
		line.start.y = start.y;
		line.end.x = start.x;
		line.end.y = end.y;
		AddLine(storage, line);
		return id;
	}
	else
	{
		TdLineRenderer2D::LineData line = { 0, start, end, color };
		return AddLine(storage, line);
	}
}

void tdVkDeleteLine(TdLineRenderer2D* renderer, uint16 id, uint32 flags)
{
	assert(renderer);
	//TdArray<LineData>& lines = (flags & TdLineRenderer2D::TD_ADDLINE_USAGE_HOST_BUFFER_BIT) ? renderer->lines_cpu : renderer->lines_gpu;

	//for (u64 i = lines.GetCount() - 1; i >= 0; i--)
	//{
	//	if (lines[i].id == id)
	//	{
	//		lines.RemoveAt(i);
	//		if (flags & TdLineRenderer2D::TD_ADDLINE_USAGE_HOST_BUFFER_BIT)
	//			renderer->vb_dirty_cpu = true; else renderer->vb_dirty_gpu = true;
	//		return;
	//	}
	//}
}

void tdVkClearLines(TdLineRenderer2D* renderer, uint32 flags)
{
	assert(renderer);
	if (flags & TdLineRenderer2D::TD_ADDLINE_USAGE_HOST_BUFFER_BIT)
	{
		tdArrayClear(renderer->lines_cpu.lines);
		renderer->lines_cpu.is_dirty = true;
	}

	if (flags & TdLineRenderer2D::TD_ADDLINE_USAGE_GPU_BUFFER_BIT)
	{
		tdArrayClear(renderer->lines_gpu.lines);
		renderer->lines_gpu.is_dirty = true;
	}
}

VkResult UpdateStorageCPU(TdLineRenderer2D* renderer, VkCommandBuffer command_buffer, TdArray<TdLineRenderer2D::VertexLine>& vertices, TdLineRenderer2D::LineStorage& storage)
{
	assert(renderer);
	assert(command_buffer);
	TdVkInstance* vulkan = renderer->vulkan;

	if (storage.vb.count)
	{
		tdVkFreeResource(*vulkan, storage.vb.buffer, storage.vb.gpu_mem, "LineRenderer2D::UpdateStorageCPU");
	}

	storage.vb.count = vertices.count;
	if (storage.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	size_t vb_size = storage.vb.count * sizeof(TdLineRenderer2D::VertexLine);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements mem_reqs;

	VkBufferCreateInfo vb_info = {};
	vb_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vb_info.size = vb_size;
	vb_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	err = vkCreateBuffer(vulkan->device, &vb_info, NULL, &storage.vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan->device, storage.vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(*vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan->device, &mem_alloc, NULL, &storage.vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan->device, storage.vb.gpu_mem, 0, mem_alloc.allocationSize, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, vertices.ptr, vb_size);
	vkUnmapMemory(vulkan->device, storage.vb.gpu_mem);

	err = vkBindBufferMemory(vulkan->device, storage.vb.buffer, storage.vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	return VK_SUCCESS;
}

VkResult UpdateStorageGPU(TdLineRenderer2D* renderer, VkCommandBuffer command_buffer, TdArray<TdLineRenderer2D::VertexLine>& vertices, TdLineRenderer2D::LineStorage& storage)
{
	assert(renderer);
	assert(command_buffer);
	TdVkInstance* vulkan = renderer->vulkan;

	if (storage.vb.count)
	{
		tdVkFreeResource(*vulkan, storage.vb.buffer, storage.vb.gpu_mem, "LineRenderer2D::UpdateStorageGPU");
	}

	storage.vb.count = vertices.count;
	if (storage.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	size_t vb_size = storage.vb.count * sizeof(TdLineRenderer2D::VertexLine);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements mem_reqs;

	TdVkBuffer stage_vb;
	VkBufferCreateInfo vb_info = {};
	vb_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vb_info.size = vb_size;
	vb_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	err = vkCreateBuffer(vulkan->device, &vb_info, NULL, &stage_vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan->device, stage_vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(*vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan->device, &mem_alloc, NULL, &stage_vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan->device, stage_vb.gpu_mem, 0, mem_alloc.allocationSize, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, vertices.ptr, vb_size);
	vkUnmapMemory(vulkan->device, stage_vb.gpu_mem);

	err = vkBindBufferMemory(vulkan->device, stage_vb.buffer, stage_vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	// Create the destination buffer with device only visibility
	vb_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	err = vkCreateBuffer(vulkan->device, &vb_info, NULL, &storage.vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan->device, storage.vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(*vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan->device, &mem_alloc, NULL, &storage.vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	err = vkBindBufferMemory(vulkan->device, storage.vb.buffer, storage.vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	VkBufferCopy copy_region = {};
	copy_region.size = vb_size;
	vkCmdCopyBuffer(command_buffer, stage_vb.buffer, storage.vb.buffer, 1, &copy_region);

	tdVkFreeResource(*vulkan, stage_vb.buffer, stage_vb.gpu_mem, "LineRenderer2D::UpdateStorageGPU_Stage");
	return VK_SUCCESS;
}

void BuildVertices(TdLineRenderer2D* renderer, TdArray<TdLineRenderer2D::LineData>& lines)
{
	assert(renderer);
	u64 count = lines.count;
	if (count < 1) return;

	uint32 vp_width = renderer->vulkan->surface_width;
	uint32 vp_height = renderer->vulkan->surface_height;

	TdLineRenderer2D::LineData* line = lines.ptr;
	TdLineRenderer2D::VertexLine v;

	for (u64 i = 0; i < count; ++i)
	{
		v.color.r = line->color.r;
		v.color.g = line->color.g;
		v.color.b = line->color.b;
		v.color.a = line->color.a;
		v.pos.x = line->start.x / vp_width * 2.0f - 1.0f;
		v.pos.y = line->start.y / vp_height * 2.0f - 1.0f;
		tdArrayAdd(renderer->vertices, v);
		v.pos.x = line->end.x / vp_width * 2.0f - 1.0f;
		v.pos.y = line->end.y / vp_height * 2.0f - 1.0f;
		tdArrayAdd(renderer->vertices, v);
		line++;
	}
}

void tdVkLineRendererPresent(TdLineRenderer2D* renderer, VkCommandBuffer command_buffer)
{
	assert(renderer);
	if (renderer->lines_cpu.is_dirty)
	{
		BuildVertices(renderer, renderer->lines_cpu.lines);
		UpdateStorageCPU(renderer, command_buffer, renderer->vertices, renderer->lines_cpu);
		renderer->lines_cpu.is_dirty = false;
		tdArrayClear(renderer->vertices);
	}

	if (renderer->lines_gpu.is_dirty)
	{
		BuildVertices(renderer, renderer->lines_gpu.lines);
		UpdateStorageGPU(renderer, command_buffer, renderer->vertices, renderer->lines_gpu);
		renderer->lines_gpu.is_dirty = false;
		tdArrayClear(renderer->vertices);
	}

	if (renderer->lines_cpu.vb.count > 0 || renderer->lines_gpu.vb.count > 0)
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline);

		VkDeviceSize offsets[1] = { 0 };
		uint32 vert_count = renderer->lines_cpu.vb.count;
		if (vert_count)
		{
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer->lines_cpu.vb.buffer, offsets);
			vkCmdDraw(command_buffer, vert_count, 1, 0, 0);
#ifdef _PROFILE_
			++draw_calls;
#endif
		}

		vert_count = renderer->lines_gpu.vb.count;
		if (vert_count)
		{
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer->lines_gpu.vb.buffer, offsets);
			vkCmdDraw(command_buffer, vert_count, 1, 0, 0);
#ifdef _PROFILE_
			++draw_calls;
#endif
		}
	}
}

VkResult tdVkLoadContent(TdLineRenderer2D* renderer, TdVkInstance* vulkan, uint32 max_vertex_count, uint32 line_width)
{
	assert(renderer);
	assert(vulkan);

	VkResult err;
	renderer->vulkan = vulkan;
	tdArrayInit(renderer->vertices, max_vertex_count);

	renderer->lines_cpu.vb.count = 0;
	renderer->lines_gpu.vb.count = 0;
	tdArrayInit(renderer->lines_cpu.lines, max_vertex_count / 2);
	tdArrayInit(renderer->lines_gpu.lines, max_vertex_count / 2);

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	err = vkCreatePipelineLayout(vulkan->device, &pipeline_layout_info, NULL, &renderer->pipeline_layout);
	if (err)
	{
		tdDisplayError("vkCreatePipelineLayout", err);
		return err;
	}

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.layout = renderer->pipeline_layout;
	pipeline_create_info.renderPass = vulkan->render_pass;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
	input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	VkPipelineRasterizationStateCreateInfo rasterization_state = {};
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.cullMode = VK_CULL_MODE_NONE;
	rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state.depthClampEnable = VK_FALSE;
	rasterization_state.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state.depthBiasEnable = VK_TRUE;
	rasterization_state.lineWidth = line_width;

	VkPipelineColorBlendAttachmentState blend_attachment_state[1] = {};
	blend_attachment_state[0].colorWriteMask = 0xf;
	blend_attachment_state[0].blendEnable = VK_TRUE;
	blend_attachment_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment_state[0].colorBlendOp = VK_BLEND_OP_ADD;
	blend_attachment_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
	blend_attachment_state[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state = {};
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.attachmentCount = 1;
	color_blend_state.pAttachments = blend_attachment_state;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	TdArray1<VkDynamicState> dynamic_state_enables;
	dynamic_state_enables.Add(VK_DYNAMIC_STATE_VIEWPORT);
	dynamic_state_enables.Add(VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pDynamicStates = dynamic_state_enables.ptr();
	dynamic_state.dynamicStateCount = dynamic_state_enables.GetCount();

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
	depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state.depthTestEnable = VK_TRUE;
	depth_stencil_state.depthWriteEnable = VK_TRUE;
	depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depth_stencil_state.stencilTestEnable = VK_FALSE;
	depth_stencil_state.front = depth_stencil_state.back;

	VkPipelineMultisampleStateCreateInfo multisample_state = {};
	multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineShaderStageCreateInfo shader_stages[2];
	shader_stages[0] = tdVkLoadShader(*vulkan, "TdLineRenderer2D.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[1] = tdVkLoadShader(*vulkan, "TdLineRenderer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	tdArrayInit(renderer->shader_modules, 2);
	tdArrayAdd(renderer->shader_modules, shader_stages[0].module);
	tdArrayAdd(renderer->shader_modules, shader_stages[1].module);

	// Define vertex layout
	VkVertexInputBindingDescription binding_descs = {};
	binding_descs.stride = sizeof(TdLineRenderer2D::VertexLine);
	binding_descs.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	TdArray1<VkVertexInputAttributeDescription> attribute_descs(2);
	VkVertexInputAttributeDescription attribute_desc = {};
	// Location 0 : Position
	attribute_desc.format = VK_FORMAT_R32G32_SFLOAT;
	attribute_descs.Add(attribute_desc);
	// Location 1 : Color
	attribute_desc.location = 1;
	attribute_desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribute_desc.offset = sizeof(float) * 2;
	attribute_descs.Add(attribute_desc);

	VkPipelineVertexInputStateCreateInfo vertex_input_state;
	vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state.vertexBindingDescriptionCount = 1;
	vertex_input_state.pVertexBindingDescriptions = &binding_descs;
	vertex_input_state.vertexAttributeDescriptionCount = attribute_descs.GetCount();
	vertex_input_state.pVertexAttributeDescriptions = attribute_descs.ptr();

	// Create PSO
	pipeline_create_info.stageCount = 2;
	pipeline_create_info.pStages = shader_stages;
	pipeline_create_info.pVertexInputState = &vertex_input_state;
	pipeline_create_info.pInputAssemblyState = &input_assembly_state;
	pipeline_create_info.pRasterizationState = &rasterization_state;
	pipeline_create_info.pColorBlendState = &color_blend_state;
	pipeline_create_info.pMultisampleState = &multisample_state;
	pipeline_create_info.pViewportState = &viewport_state;
	pipeline_create_info.pDepthStencilState = &depth_stencil_state;
	pipeline_create_info.renderPass = vulkan->render_pass;
	pipeline_create_info.pDynamicState = &dynamic_state;

	err = vkCreateGraphicsPipelines(vulkan->device, vulkan->pipeline_cache, 1, &pipeline_create_info, NULL, &renderer->pipeline);
	if (err)
	{
		tdDisplayError("vkCreateGraphicsPipelines", err);
		return err;
	}

	return VK_SUCCESS;
}

}