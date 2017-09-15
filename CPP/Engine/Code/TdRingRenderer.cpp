#include "TdRingRenderer.h"

namespace eng {

void tdVkAddRing(TdRingRenderer& renderer, uint16 id, Vector3 pos, Vector3 rot, float radius, float width, uint16 segs, Color color)
{
	if (radius <= 0.0f) return;
	TdRingRenderer::RingData ring = { id, max<uint16>(3, segs), pos, rot, radius, width, color };
	renderer.rings.Add(ring);
	renderer.vb_dirty = true;
}

void tdVkDeleteRing(TdRingRenderer& renderer, uint16 id)
{
	for (u64 i = renderer.rings.GetCount() - 1; i >= 0; i--)
	{
		if (renderer.rings[i].id == id)
		{
			renderer.rings.RemoveAt(i);
			renderer.vb_dirty = true;
			return;
		}
	}
}

VkResult StoreInVB(TdRingRenderer& renderer, VkCommandBuffer& command_buffer)
{
	renderer.vb.count = renderer.vertices.GetCount();
	if (renderer.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;

	size_t vb_size = renderer.vb.count * sizeof(TdRingRenderer::VertexRing);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements mem_reqs;

	struct StagingBuffer
	{
		VkDeviceMemory gpu_mem;
		VkBuffer buffer;
	};

	StagingBuffer stage_vb;

	// Copy Vertex Buffer to GPU
	VkBufferCreateInfo vb_info = {};
	vb_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vb_info.size = vb_size;
	vb_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	err = vkCreateBuffer(vulkan.device, &vb_info, NULL, &stage_vb.buffer);
	if (err)
	{
		tdDisplayError("vkAllocateCommandBuffers", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, stage_vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &stage_vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan.device, stage_vb.gpu_mem, 0, mem_alloc.allocationSize, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, renderer.vertices.ptr(), vb_size);
	vkUnmapMemory(vulkan.device, stage_vb.gpu_mem);

	err = vkBindBufferMemory(vulkan.device, stage_vb.buffer, stage_vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	// Create the destination buffer with device only visibility
	vb_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	err = vkCreateBuffer(vulkan.device, &vb_info, NULL, &renderer.vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, renderer.vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &renderer.vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	err = vkBindBufferMemory(vulkan.device, renderer.vb.buffer, renderer.vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	VkBufferCopy copy_region = {};
	copy_region.size = vb_size;
	vkCmdCopyBuffer(command_buffer, stage_vb.buffer, renderer.vb.buffer, 1, &copy_region);

	return VK_SUCCESS;
}

void BuildVertices(TdRingRenderer& renderer, VkCommandBuffer& command_buffer)
{
	int count = renderer.rings.GetCount();
	if (count < 1) return;

	TdRingRenderer::RingData* ring = renderer.rings.ptr();
	TdRingRenderer::VertexRing v;
	v.pos.w = 1;
	Vector3 p1(0);
	Vector3 p2(0);

	while (count--)
	{
		v.color.r = ring->color.r;
		v.color.g = ring->color.g;
		v.color.b = ring->color.b;
		v.color.a = ring->color.a;
		float a = 0.0f;
		float a_inc = pi<float>() * 2 / ring->segs;
		float w2 = ring->width * 0.5f;
		p1.x = p1.y = p2.x = p2.y = 0;
		p1.z = -ring->radius + w2;
		p2.z = -ring->radius - w2;

		for (u64 s = 0; s < ring->segs; s++)
		{
			v.pos.x = p1.x + ring->pos.x;
			v.pos.y = p1.y + ring->pos.y;
			v.pos.z = p1.z + ring->pos.z;
			renderer.vertices.Add(v);
			v.pos.x = p2.x + ring->pos.x;
			v.pos.y = p2.y + ring->pos.y;
			v.pos.z = p2.z + ring->pos.z;
			renderer.vertices.Add(v);
			a += a_inc;
			p1 = rotateY(Vector3(0, 0, -ring->radius + w2), a);
			p2 = rotateY(Vector3(0, 0, -ring->radius - w2), a);
			v.pos.x = p2.x + ring->pos.x;
			v.pos.y = p2.y + ring->pos.y;
			v.pos.z = p2.z + ring->pos.z;
			renderer.vertices.Add(v);
			v.pos.x = p1.x + ring->pos.x;
			v.pos.y = p1.y + ring->pos.y;
			v.pos.z = p1.z + ring->pos.z;
			renderer.vertices.Add(v);
		}

		ring++;
	}

	StoreInVB(renderer, command_buffer);
	renderer.vertices.Clear();
}

void tdVkRingRendererPresent(TdRingRenderer& renderer, VkCommandBuffer command_buffer, const Matrix& view_proj)
{
	if (renderer.vb_dirty)
	{
		BuildVertices(renderer, command_buffer);
		renderer.vb_dirty = false;
	}

	if (renderer.vb.count > 0)
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline_layout, 0, 1, &renderer.descriptor_set, 0, NULL);

		// Update uniform buffers
		renderer.ubo_cpu.view_proj = view_proj;
		tdVkUpdateMappableBuffer(renderer.vulkan.device, renderer.ubo, 0, sizeof(renderer.ubo_cpu), 0, &renderer.ubo_cpu);

		VkDeviceSize offsets[1] = { 0 };
		uint32 index_count = renderer.vb.count / 4 * 6;
		tdVkBindIndexBufferTri(renderer.vulkan, command_buffer, index_count);
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer.vb.buffer, offsets);
		vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
#ifdef _PROFILE_
		++draw_calls;
#endif
	}
}

void tdVkRingRendererClear(TdRingRenderer& renderer)
{
	renderer.vertices.Clear();
	renderer.rings.Clear();
	if (renderer.vb.count) tdVkFreeResource(renderer.vulkan, renderer.vb, "tdVkRingRendererClear");
	renderer.vb.count = 0;
	renderer.vb_dirty = true;
}

VkResult tdVkLoadContent(TdRingRenderer& renderer)
{
	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;
	renderer.vb.count = 0;
	renderer.vb.buffer = VK_NULL_HANDLE;
	renderer.vb.gpu_mem = VK_NULL_HANDLE;

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

	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
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
	input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rasterization_state = {};
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.cullMode = VK_CULL_MODE_NONE;
	rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state.depthClampEnable = VK_FALSE;
	rasterization_state.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state.depthBiasEnable = VK_FALSE;

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
	shader_stages[0] = tdVkLoadShader(vulkan, "TdRingRenderer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[1] = tdVkLoadShader(vulkan, "TdRingRenderer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	renderer.shader_modules.Add(shader_stages[0].module);
	renderer.shader_modules.Add(shader_stages[1].module);

	// Define vertex layout
	VkVertexInputBindingDescription binding_descs = {};
	binding_descs.stride = sizeof(TdRingRenderer::VertexRing);
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