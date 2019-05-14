#include "TdLineRenderer.h"

namespace eng {

uint16 AddLine(TdLineRenderer::LineStorage& storage, TdLineRenderer::LineData& line)
{
	line.id = (uint16)storage.lines.GetCount() + 1;
	storage.lines.Add(line);
	storage.is_dirty = true;
	return line.id;
}

uint16 tdVkAddLine(TdLineRenderer& renderer, uint32 flags, Vector3 start, Vector3 end, Color color)
{
	TdLineRenderer::LineStorage& storage = (flags & TdLineRenderer::TD_ADDLINE_USAGE_HOST_BUFFER_BIT)
		? renderer.lines_cpu : renderer.lines_gpu;

	if (flags & TdLineRenderer::TD_ADDLINE_USAGE_CUBE_OUTLINE_BIT)
	{
		tdVector3MinMax(start, end);
		TdLineRenderer::LineData line = { 0, Vector3(start.x, start.y, start.z), Vector3(end.x, start.y, start.z), color };
		uint16 id = AddLine(storage, line);
		line.start.x = end.x;
		line.end.z = end.z;
		AddLine(storage, line);
		line.start.z = end.z;
		line.end.x = start.x;
		AddLine(storage, line);
		line.start.x = start.x;
		line.end.z = start.z;
		AddLine(storage, line);

		line.start.z = start.z;
		line.end.y = end.y;
		AddLine(storage, line);
		line.start.x = end.x;
		line.end.x = end.x;
		AddLine(storage, line);
		line.start.z = end.z;
		line.end.z = end.z;
		AddLine(storage, line);
		line.start.x = start.x;
		line.end.x = start.x;
		AddLine(storage, line);

		line.start.z = start.z;
		line.end.z = start.z;
		line.start.y = end.y;
		line.end.x = end.x;
		AddLine(storage, line);
		line.start.x = end.x;
		line.end.z = end.z;
		AddLine(storage, line);
		line.start.z = end.z;
		line.end.x = start.x;
		AddLine(storage, line);
		line.start.x = start.x;
		line.end.z = start.z;
		AddLine(storage, line);

		return id;
	}
	else
	{
		TdLineRenderer::LineData line = { 0, start, end, color };
		return AddLine(storage, line);
	}
}

void tdVkDeleteLine(TdLineRenderer& renderer, uint16 id, uint32 flags)
{
	//TdArray<LineData>& lines = (flags & TdLineRenderer::TD_ADDLINE_USAGE_HOST_BUFFER_BIT) ? renderer.lines_cpu : renderer.lines_gpu;

	//for (u64 i = lines.GetCount() - 1; i >= 0; i--)
	//{
	//	if (lines[i].id == id)
	//	{
	//		lines.RemoveAt(i);
	//		if (flags & TdLineRenderer::TD_ADDLINE_USAGE_HOST_BUFFER_BIT)
	//			renderer.vb_dirty_cpu = true; else renderer.vb_dirty_gpu = true;
	//		return;
	//	}
	//}
}

void tdVkClearLines(TdLineRenderer& renderer, uint32 flags)
{
	if (flags & TdLineRenderer::TD_ADDLINE_USAGE_HOST_BUFFER_BIT)
	{
		renderer.lines_cpu.lines.Clear();
		renderer.lines_cpu.is_dirty = true;
	}

	if (flags & TdLineRenderer::TD_ADDLINE_USAGE_GPU_BUFFER_BIT)
	{
		renderer.lines_gpu.lines.Clear();
		renderer.lines_gpu.is_dirty = true;
	}
}

VkResult UpdateStorageCPU(TdLineRenderer& renderer, VkCommandBuffer& command_buffer, TdArray<TdLineRenderer::VertexLine>& vertices, TdLineRenderer::LineStorage& storage)
{
	TdVkInstance* vulkan = renderer.vulkan;
	if (storage.vb.count)
	{
		tdVkFreeResource(vulkan, storage.vb.buffer, storage.vb.gpu_mem, "LineRenderer::UpdateStorageCPU");
	}

	storage.vb.count = vertices.count;
	if (storage.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	size_t vb_size = storage.vb.count * sizeof(TdLineRenderer::VertexLine);

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
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
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

VkResult UpdateStorageGPU(TdLineRenderer& renderer, VkCommandBuffer& command_buffer, TdArray<TdLineRenderer::VertexLine>& vertices, TdLineRenderer::LineStorage& storage)
{
	TdVkInstance* vulkan = renderer.vulkan;
	if (storage.vb.count)
	{
		tdVkFreeResource(vulkan, storage.vb.buffer, storage.vb.gpu_mem, "LineRenderer::UpdateStorageGPU");
	}

	storage.vb.count = vertices.count;
	if (storage.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	size_t vb_size = storage.vb.count * sizeof(TdLineRenderer::VertexLine);

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
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
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
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
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

	tdVkFreeResource(vulkan, stage_vb.buffer, stage_vb.gpu_mem, "LineRenderer::UpdateStorageGPU_Stage");
	return VK_SUCCESS;
}

void BuildVertices(TdArray1<TdLineRenderer::LineData>& lines, TdArray<TdLineRenderer::VertexLine>& vertices)
{
	int count = lines.GetCount();
	if (count < 1) return;

	TdLineRenderer::LineData* line = lines.ptr();
	TdLineRenderer::VertexLine v;
	v.pos.w = 1;

	for (u64 i = 0; i < count; ++i)
	{
		v.color.r = line->color.r;
		v.color.g = line->color.g;
		v.color.b = line->color.b;
		v.color.a = line->color.a;
		v.pos.x = line->start.x;
		v.pos.y = line->start.y;
		v.pos.z = line->start.z;
		tdArrayAdd(vertices, v);
		v.pos.x = line->end.x;
		v.pos.y = line->end.y;
		v.pos.z = line->end.z;
		tdArrayAdd(vertices, v);
		line++;
	}
}

void tdVkLineRendererPresent(TdLineRenderer& renderer, VkCommandBuffer command_buffer, const Matrix& view_proj, Vector3 cam_pos, float far_clip)
{
	if (renderer.lines_cpu.is_dirty)
	{
		BuildVertices(renderer.lines_cpu.lines, renderer.vertices);
		UpdateStorageCPU(renderer, command_buffer, renderer.vertices, renderer.lines_cpu);
		renderer.lines_cpu.is_dirty = false;
		tdArrayClear(renderer.vertices);
	}

	if (renderer.lines_gpu.is_dirty)
	{
		BuildVertices(renderer.lines_gpu.lines, renderer.vertices);
		UpdateStorageGPU(renderer, command_buffer, renderer.vertices, renderer.lines_gpu);
		renderer.lines_gpu.is_dirty = false;
		tdArrayClear(renderer.vertices);
	}

	if (renderer.lines_cpu.vb.count > 0 || renderer.lines_gpu.vb.count > 0)
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline_layout, 0, 1, &renderer.descriptor_set, 0, NULL);

		renderer.ubo_cpu.view_proj = view_proj;
		renderer.ubo_cpu.cam_pos = cam_pos;
		renderer.ubo_cpu.far_clip = far_clip;
		renderer.ubo.cpu_mem = &renderer.ubo_cpu;
		tdVkUpdateMappableBuffer(renderer.vulkan->device, renderer.ubo, 0, sizeof(renderer.ubo_cpu), 0);

		VkDeviceSize offsets[1] = { 0 };

		uint32 vert_count = renderer.lines_cpu.vb.count;
		if (vert_count)
		{
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer.lines_cpu.vb.buffer, offsets);
			vkCmdDraw(command_buffer, vert_count, 1, 0, 0);
#ifdef _PROFILE_
			++draw_calls;
#endif
		}

		vert_count = renderer.lines_gpu.vb.count;
		if (vert_count)
		{
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer.lines_gpu.vb.buffer, offsets);
			vkCmdDraw(command_buffer,  vert_count, 1, 0, 0);
#ifdef _PROFILE_
			++draw_calls;
#endif
		}
	}
}

VkResult tdVkLoadContent(TdLineRenderer& renderer, TdVkInstance& vulkan, uint32 max_vertex_count, uint32 line_width)
{
	VkResult err;
	renderer.vulkan = &vulkan;
	tdArrayInit(renderer.vertices, max_vertex_count);

	renderer.lines_cpu.vb.count = 0;
	renderer.lines_gpu.vb.count = 0;

	// Uniform Buffers
	VkBufferCreateInfo buff_info = {};
	buff_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buff_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buff_info.size = sizeof(renderer.ubo_cpu);

	err = vkCreateBuffer(vulkan.device, &buff_info, NULL, &renderer.ubo.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	VkMemoryRequirements mem_reqs = {};
	vkGetBufferMemoryRequirements(vulkan.device, renderer.ubo.buffer, &mem_reqs);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;

	if (!tdVkGetMemoryType(&vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	// Allocate memory for the uniform buffer
	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &renderer.ubo.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	// Bind memory to buffer
	err = vkBindBufferMemory(vulkan.device, renderer.ubo.buffer, renderer.ubo.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	// Set Descriptor Bindings
	VkDescriptorSetLayoutBinding layout_binding[1];
	layout_binding[0] = {};
	layout_binding[0].binding = 0;
	layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding[0].descriptorCount = 1;
	layout_binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_binding[0].pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptor_layout_info = {};
	descriptor_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout_info.bindingCount = 1;
	descriptor_layout_info.pBindings = layout_binding;

	VkDescriptorSetLayout descriptor_set_layout;
	err = vkCreateDescriptorSetLayout(vulkan.device, &descriptor_layout_info, NULL, &descriptor_set_layout);
	if (err)
	{
		tdDisplayError("vkCreateDescriptorSetLayout", err);
		return err;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descriptor_set_layout;

	err = vkCreatePipelineLayout(vulkan.device, &pipeline_layout_info, NULL, &renderer.pipeline_layout);
	if (err)
	{
		tdDisplayError("vkCreatePipelineLayout", err);
		return err;
	}

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.layout = renderer.pipeline_layout;
	pipeline_create_info.renderPass = vulkan.render_pass;

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
	shader_stages[0] = tdVkLoadShader(&vulkan, "TdLineRenderer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[1] = tdVkLoadShader(&vulkan, "TdLineRenderer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	renderer.shader_modules.Add(shader_stages[0].module);
	renderer.shader_modules.Add(shader_stages[1].module);

	// Define vertex layout
	VkVertexInputBindingDescription binding_descs = {};
	binding_descs.stride = sizeof(TdLineRenderer::VertexLine);
	binding_descs.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	TdArray1<VkVertexInputAttributeDescription> attribute_descs(2);
	VkVertexInputAttributeDescription attribute_desc = {};
	// Location 0 : Position
	attribute_desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribute_descs.Add(attribute_desc);
	// Location 1 : Color
	attribute_desc.location = 1;
	attribute_desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attribute_desc.offset = sizeof(float) * 4;
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
	pipeline_create_info.renderPass = vulkan.render_pass;
	pipeline_create_info.pDynamicState = &dynamic_state;

	err = vkCreateGraphicsPipelines(vulkan.device, vulkan.pipeline_cache, 1, &pipeline_create_info, NULL, &renderer.pipeline);
	if (err)
	{
		tdDisplayError("vkCreateGraphicsPipelines", err);
		return err;
	}

	VkDescriptorPoolSize type_counts[1];
	type_counts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	type_counts[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool_info = {};
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.poolSizeCount = 1;
	descriptor_pool_info.pPoolSizes = type_counts;
	descriptor_pool_info.maxSets = 1;

	err = vkCreateDescriptorPool(vulkan.device, &descriptor_pool_info, NULL, &renderer.descriptor_pool);
	if (err)
	{
		tdDisplayError("vkCreateDescriptorPool", err);
		return err;
	}

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = renderer.descriptor_pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &descriptor_set_layout;

	err = vkAllocateDescriptorSets(vulkan.device, &alloc_info, &renderer.descriptor_set);
	if (err)
	{
		tdDisplayError("vkAllocateDescriptorSets", err);
		return err;
	}

	VkDescriptorBufferInfo descriptor = {};
	descriptor.buffer = renderer.ubo.buffer;
	descriptor.range = sizeof(renderer.ubo_cpu);
	descriptor.offset = 0;

	VkWriteDescriptorSet write_descriptor_set = {};
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.dstSet = renderer.descriptor_set;
	write_descriptor_set.descriptorCount = 1;
	write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_set.pBufferInfo = &descriptor;
	write_descriptor_set.dstBinding = 0;
	vkUpdateDescriptorSets(vulkan.device, 1, &write_descriptor_set, 0, NULL);

	return VK_SUCCESS;
}

}