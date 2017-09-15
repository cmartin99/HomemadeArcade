#include "TdVkInstance.h"
#include <iostream>
#include <array>

namespace eng {

VkEvent tdVkCreateEvent(TdVkInstance& vulkan)
{
	VkEvent result;
	VkEventCreateInfo info = { VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
	VkResult err = vkCreateEvent(vulkan.device, &info, NULL, &result);
	if (err)
	{
		tdDisplayError("vkCreateEvent", err);
		return NULL;
	}
	return result;
}

VkFence tdVkCreateFence(TdVkInstance& vulkan, VkFenceCreateFlags flags)
{
	VkFence result;
	VkFenceCreateInfo info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, flags };
	VkResult err = vkCreateFence(vulkan.device, &info, NULL, &result);
	if (err)
	{
		tdDisplayError("vkCreateFence", err);
		return NULL;
	}
	return result;
}

VkResult tdVkWaitForFence(TdVkInstance& vulkan, VkFence fence)
{
	VkResult result = VK_TIMEOUT;
	while (result == VK_TIMEOUT);
	{
		result = vkWaitForFences(vulkan.device, 1, &fence, VK_TRUE, 100000000);
	}
	return result;
}

VkResult tdVkWaitForFrameFence(TdVkInstance& vulkan)
{
	VkFence fence = vulkan.frames[vulkan.current_frame].fence;
	VkResult result = vkGetFenceStatus(vulkan.device, fence);
	if (result == VK_NOT_READY)
	{
		result = tdVkWaitForFence(vulkan, fence);
		if (result != VK_SUCCESS) return result;
	}
	return vkResetFences(vulkan.device, 1, &fence);
}

VkResult tdVkUpdateMappableBuffer(VkCommandBuffer command_buffer, TdVkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, void* cpu_buffer)
{
	vkCmdUpdateBuffer(command_buffer, buffer.buffer, offset, size, (uint32*)cpu_buffer);
	return VK_SUCCESS;
}

VkResult tdVkUpdateMappableBuffer(VkDevice device, TdVkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void* cpu_buffer)
{
	VkResult err;
	void* cpu_mem;

	err = vkMapMemory(device, buffer.gpu_mem, offset, size, flags, (void **)&cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, cpu_buffer, size);

	vkUnmapMemory(device, buffer.gpu_mem);
	if (err)
	{
		tdDisplayError("vkUnmapMemory", err);
		return err;
	}

	return VK_SUCCESS;
}

std::string tdErrorString(VkResult errorCode)
{
	switch (errorCode)
	{
#define STR(r) case VK_ ##r: return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}

VkShaderModule LoadShader(const char *file_name, VkDevice device, VkShaderStageFlagBits stage)
{
	char full_path[500];
	memset(full_path, 0, 500);
	if (shader_path) strncpy(full_path, shader_path, 400);
	strncat(full_path, file_name, 500 - strlen(full_path));

	size_t size = 0;
	const char *shaderCode = tdReadBinaryFile(full_path, &size);
	assert(size > 0);

	VkResult err;
	VkShaderModule shader_module;
	VkShaderModuleCreateInfo module_create_info = {};
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.codeSize = size;
	module_create_info.pCode = (uint32*)shaderCode;

	err = vkCreateShaderModule(device, &module_create_info, NULL, &shader_module);
	if (err)
	{
		tdDisplayError("vkCreateShaderModule", err);
		return 0;
	}

	return shader_module;
}

VkPipelineShaderStageCreateInfo tdVkLoadShader(TdVkInstance& vulkan, const char* file_name, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shader_stage = {};
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = stage;
	shader_stage.module = LoadShader(file_name, vulkan.device, stage);

	if (!shader_stage.module)
	{
		tdDisplayError("LoadShader", VK_ERROR_INITIALIZATION_FAILED);
		return shader_stage;
	}

	shader_stage.pName = "main"; // todo : make param
	assert(shader_stage.module != NULL);
	return shader_stage;
}

VkBool32 tdVkGetMemoryType(TdVkInstance& vulkan, uint32 type_bits, VkFlags properties, uint32* type_index)
{
	for (u64 i = 0; i < vulkan.device_memory_properties.memoryTypeCount; ++i)
	{
		if ((type_bits & 1) == 1)
		{
			if ((vulkan.device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				*type_index = i;
				return true;
			}
		}
		type_bits >>= 1;
	}
	return false;
}

void tdVkSetImageLayout(VkCommandBuffer command_buffer, VkImage image, VkImageAspectFlags aspect_mask, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkImageSubresourceRange subresource_range)
{
	VkImageMemoryBarrier image_mem_barrier = {};
	image_mem_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_mem_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_mem_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_mem_barrier.oldLayout = old_image_layout;
	image_mem_barrier.newLayout = new_image_layout;
	image_mem_barrier.image = image;
	image_mem_barrier.subresourceRange = subresource_range;

	// Source layouts (old)

	// Undefined layout
	// Only allowed as initial layout!
	// Make sure any writes to the image have been finished
	if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
	{
		image_mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	// Old layout is color attachment
	// Make sure any writes to the color buffer have been finished
	if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		image_mem_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	// Old layout is depth/stencil attachment
	// Make sure any writes to the depth/stencil buffer have been finished
	if (old_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		image_mem_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	// Old layout is transfer source
	// Make sure any reads from the image have been finished
	if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		image_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// Old layout is shader read (sampler, input attachment)
	// Make sure any shader reads from the image have been finished
	if (old_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		image_mem_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	// Target layouts (new)

	// New layout is transfer destination (copy, blit)
	// Make sure any copyies to the image have been finished
	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		image_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	// New layout is transfer source (copy, blit)
	// Make sure any reads from and writes to the image have been finished
	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		image_mem_barrier.srcAccessMask = image_mem_barrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		image_mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// New layout is color attachment
	// Make sure any writes to the color buffer hav been finished
	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		image_mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		image_mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// New layout is depth attachment
	// Make sure any writes to depth/stencil buffer have been finished
	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		image_mem_barrier.dstAccessMask = image_mem_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	// New layout is shader read (sampler, input attachment)
	// Make sure any writes to the image have been finished
	if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		image_mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		image_mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	// Put barrier on top
	VkPipelineStageFlags src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	vkCmdPipelineBarrier(command_buffer, src_stage_flags, dest_stage_flags, 0, 0, NULL, 0, NULL, 1, &image_mem_barrier);
}

int ChoosePhysicalDeviceIndex(VkPhysicalDevice* devices)
{
	// TODO: choose device index
	return 0;
}

VkResult UploadIndexBufferTri(TdVkInstance& vulkan, VkCommandBuffer command_buffer, uint32 index_count)
{
	VkResult err;
	TdArray1<uint32> indices(index_count);
	uint32 face_count = index_count / 6;

	for (uint32 i = 0; i < face_count; ++i)
	{
		indices.Add(i * 4);
		indices.Add(i * 4 + 1);
		indices.Add(i * 4 + 2);
		indices.Add(i * 4);
		indices.Add(i * 4 + 2);
		indices.Add(i * 4 + 3);
	}

	struct Op
	{
		TdVkInstance& vulkan;
		VkBuffer stage_buffer;
		VkDeviceMemory stage_gpu_mem;
		VkBuffer index_buffer;
		VkDeviceMemory index_gpu_mem;

		Op(TdVkInstance& vulkan) : vulkan(vulkan), stage_buffer(NULL), stage_gpu_mem(NULL), index_buffer(NULL), index_gpu_mem(NULL) {}
		void Cleanup()
		{
			if (stage_buffer) vkDestroyBuffer(vulkan.device, stage_buffer, NULL);
			if (stage_gpu_mem) vkFreeMemory(vulkan.device, stage_gpu_mem, NULL);
			if (index_buffer) vkDestroyBuffer(vulkan.device, index_buffer, NULL);
			if (index_gpu_mem) vkFreeMemory(vulkan.device, index_gpu_mem, NULL);
			if (vulkan.ib_tri.count)
			{
				tdVkFreeResource(vulkan, vulkan.ib_tri.buffer, vulkan.ib_tri.gpu_mem, "UploadIndexBufferTri");
				vulkan.ib_tri.count = 0;
			}
		}
	};

	Op op(vulkan);
	size_t ib_size = index_count * sizeof(uint32);

	// Index buffer
	VkBufferCreateInfo ib_info = {};
	ib_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ib_info.size = ib_size;
	ib_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &op.stage_buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		op.Cleanup();
		return err;
	}

	VkMemoryRequirements mem_reqs = {};
	vkGetBufferMemoryRequirements(vulkan.device, op.stage_buffer, &mem_reqs);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;

	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		op.Cleanup();
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &op.stage_gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		op.Cleanup();
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan.device, op.stage_gpu_mem, 0, ib_size, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		op.Cleanup();
		return err;
	}

	memcpy(cpu_mem, indices.ptr(), ib_size);
	vkUnmapMemory(vulkan.device, op.stage_gpu_mem);

	err = vkBindBufferMemory(vulkan.device, op.stage_buffer, op.stage_gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		op.Cleanup();
		return err;
	}

	ib_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &op.index_buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		op.Cleanup();
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, op.index_buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		op.Cleanup();
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &op.index_gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		op.Cleanup();
		return err;
	}

	err = vkBindBufferMemory(vulkan.device, op.index_buffer, op.index_gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		op.Cleanup();
		return err;
	}

	VkBufferCopy copy_region = {};
	copy_region.size = ib_size;
	vkCmdCopyBuffer(command_buffer, op.stage_buffer, op.index_buffer, 1, &copy_region);

	if (vulkan.ib_tri.count)
	{
		tdVkFreeResource(vulkan, vulkan.ib_tri.buffer, vulkan.ib_tri.gpu_mem, "UploadIndexBufferTri");
	}

	vulkan.ib_tri.count = index_count;
	vulkan.ib_tri.buffer = op.index_buffer;
	vulkan.ib_tri.gpu_mem = op.index_gpu_mem;

	tdVkFreeResource(vulkan, op.stage_buffer, op.stage_gpu_mem, "UploadIndexBufferTri_Stage");
	return VK_SUCCESS;
}

VkResult UploadIndexBufferLine(TdVkInstance& vulkan, VkCommandBuffer command_buffer, uint32 index_count)
{
	VkResult err;
	TdArray1<uint32> indices(index_count);
	uint32 line_count = index_count / 2;

	for (uint32 i = 0; i < line_count; ++i)
	{
		indices.Add(i * 2);
		indices.Add(i * 2 + 1);
	}

	struct Op
	{
		TdVkInstance& vulkan;
		VkBuffer stage_buffer;
		VkDeviceMemory stage_gpu_mem;
		VkBuffer index_buffer;
		VkDeviceMemory index_gpu_mem;

		Op(TdVkInstance& vulkan) : vulkan(vulkan), stage_buffer(NULL), stage_gpu_mem(NULL), index_buffer(NULL), index_gpu_mem(NULL) {}
		void Cleanup()
		{
			if (stage_buffer) vkDestroyBuffer(vulkan.device, stage_buffer, NULL);
			if (stage_gpu_mem) vkFreeMemory(vulkan.device, stage_gpu_mem, NULL);
			if (index_buffer) vkDestroyBuffer(vulkan.device, index_buffer, NULL);
			if (index_gpu_mem) vkFreeMemory(vulkan.device, index_gpu_mem, NULL);
			if (vulkan.ib_line.count)
			{
				tdVkFreeResource(vulkan, vulkan.ib_line.buffer, vulkan.ib_line.gpu_mem, "UploadIndexBufferLine");
				vulkan.ib_line.count = 0;
			}
		}
	};

	Op op(vulkan);
	size_t ib_size = index_count * sizeof(uint32);

	// Index buffer
	VkBufferCreateInfo ib_info = {};
	ib_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ib_info.size = ib_size;
	ib_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &op.stage_buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		op.Cleanup();
		return err;
	}

	VkMemoryRequirements mem_reqs = {};
	vkGetBufferMemoryRequirements(vulkan.device, op.stage_buffer, &mem_reqs);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;

	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		op.Cleanup();
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &op.stage_gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		op.Cleanup();
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan.device, op.stage_gpu_mem, 0, ib_size, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		op.Cleanup();
		return err;
	}

	memcpy(cpu_mem, indices.ptr(), ib_size);
	vkUnmapMemory(vulkan.device, op.stage_gpu_mem);

	err = vkBindBufferMemory(vulkan.device, op.stage_buffer, op.stage_gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		op.Cleanup();
		return err;
	}

	ib_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &op.index_buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		op.Cleanup();
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, op.index_buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		op.Cleanup();
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &op.index_gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		op.Cleanup();
		return err;
	}

	err = vkBindBufferMemory(vulkan.device, op.index_buffer, op.index_gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		op.Cleanup();
		return err;
	}

	VkBufferCopy copy_region = {};
	copy_region.size = ib_size;
	vkCmdCopyBuffer(command_buffer, op.stage_buffer, op.index_buffer, 1, &copy_region);

	if (vulkan.ib_line.count)
	{
		tdVkFreeResource(vulkan, vulkan.ib_line.buffer, vulkan.ib_line.gpu_mem, "UploadIndexBufferLine");
	}

	vulkan.ib_line.count = index_count;
	vulkan.ib_line.buffer = op.index_buffer;
	vulkan.ib_line.gpu_mem = op.index_gpu_mem;

	tdVkFreeResource(vulkan, op.stage_buffer, op.stage_gpu_mem, "UploadIndexBufferLine_Staged");

	return VK_SUCCESS;
}

void tdVkBindIndexBufferTri(TdVkInstance& vulkan, VkCommandBuffer command_buffer, uint32 index_count)
{
	if (index_count > vulkan.ib_tri.count)
	{
		if (UploadIndexBufferTri(vulkan, command_buffer, index_count) < 0)
		{
			return;
		}
	}

	vkCmdBindIndexBuffer(command_buffer, vulkan.ib_tri.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void tdVkBindIndexBufferLine(TdVkInstance& vulkan, VkCommandBuffer command_buffer, uint32 index_count)
{
	if (index_count > vulkan.ib_line.count)
	{
		if (UploadIndexBufferLine(vulkan, command_buffer, index_count) < 0)
		{
			return;
		}
	}

	vkCmdBindIndexBuffer(command_buffer, vulkan.ib_line.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void tdVkFreeResource(TdVkInstance& vulkan, VkBuffer buffer, VkDeviceMemory mem, const char* name)
{
	TdVkBuffer b = { 0, buffer, mem, name };
	tdVkFreeResource(vulkan, b, name);
}

void tdVkFreeResource(TdVkInstance& vulkan, TdVkBuffer buffer, const char* name)
{
	//TdMutexLock lock(vulkan.frames[vulkan.current_frame].free_resource_mutex);

	buffer.name = name;
	vulkan.frames[vulkan.current_frame].buffer_release.Push(buffer);

	//TdArray<TdVkBuffer>& log = vulkan.frames[vulkan.current_frame].buffer_release_log;
	//for (int i = 0; i < log.GetCount(); i++) assert(log[i].gpu_mem != buffer.gpu_mem);
	//vulkan.frames[vulkan.current_frame].buffer_release_log.Push(buffer);
}

void tdVkUpdateFreeResources(TdVkInstance& vulkan)
{
	//TdMutexLock lock(vulkan.frames[vulkan.current_frame].free_resource_mutex);

	int count = vulkan.frames[vulkan.current_frame].buffer_release.GetCount();
	TdVkBuffer* buffers = vulkan.frames[vulkan.current_frame].buffer_release.ptr();

	while (count--)
	{
		if (buffers->gpu_mem) vkFreeMemory(vulkan.device, buffers->gpu_mem, NULL);
		if (buffers->buffer) vkDestroyBuffer(vulkan.device, buffers->buffer, NULL);
		++buffers;
	}

	vulkan.frames[vulkan.current_frame].buffer_release.Clear();
}

void tdVkFreeCommandBuffer(TdVkInstance& vulkan, VkQueue queue, VkCommandBuffer command_buffer)
{
	TdArray1<VkCommandBuffer>& list = (queue == vulkan.queue ? vulkan.command_buffer_free : vulkan.command_buffer_free2);
	list.Add(command_buffer);
}

void tdVkUpdateFreeCommandBuffers(TdVkInstance& vulkan, VkQueue queue)
{
	TdArray1<VkCommandBuffer>& list = (queue == vulkan.queue ? vulkan.command_buffer_free : vulkan.command_buffer_free2);
	vkFreeCommandBuffers(vulkan.device, vulkan.command_pool, list.GetCount(), list.ptr());
	list.Clear();
}

bool IsPresentModeSupported(TdArray1<VkPresentModeKHR>& supported_modes, VkPresentModeKHR desired)
{
	for (u64 i = 0; i < supported_modes.GetCount(); ++i)
	{
		if (supported_modes[i] == desired) return true;
	}
	return false;
}

VkResult InitSwapChain(TdVkInstance& vulkan, uint32 *width, uint32 *height)
{
	VkResult err;
	VkSwapchainKHR old_swap_chain = vulkan.swap_chain;
	uint32 old_frame_count = vulkan.frame_count;

	VkSurfaceCapabilitiesKHR surf_caps;
	err = vulkan.fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan.physical_device, vulkan.surface, &surf_caps);
	if (err)
	{
		tdDisplayError("fpGetPhysicalDeviceSurfaceCapabilitiesKHR", err);
		return err;
	}

	uint32 present_mode_count;
	err = vulkan.fpGetPhysicalDeviceSurfacePresentModesKHR(vulkan.physical_device, vulkan.surface, &present_mode_count, NULL);
	if (err)
	{
		tdDisplayError("fpGetPhysicalDeviceSurfacePresentModesKHR1", err);
		return err;
	}

	assert(present_mode_count > 0);
	TdArray1<VkPresentModeKHR> present_modes(present_mode_count);

	err = vulkan.fpGetPhysicalDeviceSurfacePresentModesKHR(vulkan.physical_device, vulkan.surface, &present_mode_count, present_modes.ptr());
	if (err)
	{
		tdDisplayError("fpGetPhysicalDeviceSurfacePresentModesKHR2", err);
		return err;
	}

	VkExtent2D swap_chain_extent = {};
	if (surf_caps.currentExtent.width == -1)
	{
		swap_chain_extent.width = *width;
		swap_chain_extent.height = *height;
	}
	else
	{
		swap_chain_extent = surf_caps.currentExtent;
		*width = surf_caps.currentExtent.width;
		*height = surf_caps.currentExtent.height;
	}

	VkPresentModeKHR swap_chain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	if (IsPresentModeSupported(present_modes, VK_PRESENT_MODE_MAILBOX_KHR)) swap_chain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	else if (IsPresentModeSupported(present_modes, VK_PRESENT_MODE_FIFO_KHR)) swap_chain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	else if (IsPresentModeSupported(present_modes, VK_PRESENT_MODE_FIFO_RELAXED_KHR)) swap_chain_present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;

	VkSwapchainCreateInfoKHR swap_chain_create_info = {};
	swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_create_info.surface = vulkan.surface;
	swap_chain_create_info.minImageCount = min<uint32>(max<uint32>(3, surf_caps.minImageCount), surf_caps.maxImageCount > 0 ? surf_caps.maxImageCount : 3 );
	swap_chain_create_info.imageFormat = vulkan.color_format;
	swap_chain_create_info.imageColorSpace = vulkan.color_space;
	swap_chain_create_info.imageExtent = { swap_chain_extent.width, swap_chain_extent.height };
	swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swap_chain_create_info.preTransform = (VkSurfaceTransformFlagBitsKHR)(surf_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surf_caps.currentTransform;
	swap_chain_create_info.imageArrayLayers = 1;
	swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swap_chain_create_info.presentMode = swap_chain_present_mode;
	swap_chain_create_info.oldSwapchain = old_swap_chain;
	swap_chain_create_info.clipped = true;
	swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	err = vulkan.fpCreateSwapchainKHR(vulkan.device, &swap_chain_create_info, NULL, &vulkan.swap_chain);
	if (err)
	{
		tdDisplayError("fpGetPhysicalDeviceSurfacePresentModesKHR2", err);
		return err;
	}

	if (old_swap_chain != VK_NULL_HANDLE)
	{
		for (u64 i = 0; i < old_frame_count; ++i)
		{
			vkDestroyImageView(vulkan.device, vulkan.frames[i].image_view, NULL);
		}
		vulkan.fpDestroySwapchainKHR(vulkan.device, old_swap_chain, NULL);
	}

	// New Frame Count
	err = vulkan.fpGetSwapchainImagesKHR(vulkan.device, vulkan.swap_chain, &vulkan.frame_count, NULL);
	if (err)
	{
		tdDisplayError("fpGetSwapchainImagesKHR1", err);
		return err;
	}

	vulkan.frames = new TdVkInstance::FrameData[vulkan.frame_count];
	VkImage* frame_images = (VkImage*)malloc(sizeof(VkImage) * vulkan.frame_count);

	err = vulkan.fpGetSwapchainImagesKHR(vulkan.device, vulkan.swap_chain, &vulkan.frame_count, frame_images);
	if (err)
	{
		tdDisplayError("fpGetSwapchainImagesKHR2", err);
		return err;
	}

	for (u64 i = 0; i < vulkan.frame_count; ++i)
	{
		vulkan.frames[i].image = frame_images[i];

		VkImageViewCreateInfo color_attachment_view = {};
		color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_attachment_view.format = vulkan.color_format;
		color_attachment_view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		color_attachment_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_attachment_view.subresourceRange.baseMipLevel = 0;
		color_attachment_view.subresourceRange.levelCount = 1;
		color_attachment_view.subresourceRange.baseArrayLayer = 0;
		color_attachment_view.subresourceRange.layerCount = 1;
		color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.baseMipLevel = 0;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;

		tdVkSetImageLayout(vulkan.setup_command_buffer, vulkan.frames[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresource_range);
		color_attachment_view.image = vulkan.frames[i].image;

		err = vkCreateImageView(vulkan.device, &color_attachment_view, NULL, &vulkan.frames[i].image_view);
		if (err)
		{
			tdDisplayError("vkCreateImageView", err);
			return err;
		}
	}

	free(frame_images);
	return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL tdVKDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64 object, size_t location, int32 message_code, const char* layer_prefix, const char* message, void* user_data)
{
	//std::cerr << message << std::endl;
	//tdDisplayError(message, VK_ERROR_VALIDATION_FAILED_EXT);
	tdWriteBinaryFile("error.log", (void*)message, strlen(message), true);
	return VK_FALSE;
}

VkResult tdVkInitVulkan(TdVkInstance& vulkan, const char* name, bool enable_validation, HINSTANCE hinst, HWND hwnd)
{
	VkResult err;
	vulkan.ib_tri.count = 0;
	vulkan.ib_line.count = 0;

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = name;
	app_info.pEngineName = name;
	app_info.apiVersion = VK_API_VERSION_1_0;

	int extn_count = 2;
	const char* enabled_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, nullptr};
	if (enable_validation) { enabled_extensions[2] = "VK_EXT_debug_report"; ++extn_count; }

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledExtensionCount = extn_count;
	instance_create_info.ppEnabledExtensionNames = enabled_extensions;

	const char *enabled_validation_layers[] = {"VK_LAYER_LUNARG_standard_validation"};
	if (enable_validation)
	{
		//enabled_validation_layers.Push(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		instance_create_info.enabledLayerCount = 1;
		instance_create_info.ppEnabledLayerNames = enabled_validation_layers;
	}

	err = vkCreateInstance(&instance_create_info, NULL, &vulkan.instance);
	if (err)
	{
		tdDisplayError("vkCreateInstance", err);
		return err;
	}

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
        reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
            (vkGetInstanceProcAddr(vulkan.instance, "vkCreateDebugReportCallbackEXT"));
    PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
        reinterpret_cast<PFN_vkDebugReportMessageEXT>
            (vkGetInstanceProcAddr(vulkan.instance, "vkDebugReportMessageEXT"));
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
        reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
            (vkGetInstanceProcAddr(vulkan.instance, "vkDestroyDebugReportCallbackEXT"));

	VkDebugReportCallbackCreateInfoEXT callback_info = {};
	VkDebugReportCallbackEXT callback;
	if (enable_validation)
	{
		callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		callback_info.pfnCallback = &tdVKDebugReportCallback;
		callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
							VK_DEBUG_REPORT_WARNING_BIT_EXT |
							VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;

		err = vkCreateDebugReportCallbackEXT(vulkan.instance, &callback_info, NULL, &callback);
		if (err)
		{
			tdDisplayError("vkCreateDebugReportCallbackEXT", err);
			return err;
		}
	}

	uint32 gpu_count = 0;
	err = vkEnumeratePhysicalDevices(vulkan.instance, &gpu_count, NULL);
	if (err)
	{
		tdDisplayError("vkEnumeratePhysicalDevices", err);
		return err;
	}

	assert(gpu_count > 0);

	VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
	err = vkEnumeratePhysicalDevices(vulkan.instance, &gpu_count, physical_devices);
	if (err)
	{
		tdDisplayError("vkEnumeratePhysicalDevices", err);
		return err;
	}

	vulkan.physical_device = physical_devices[ChoosePhysicalDeviceIndex(physical_devices)];

	uint32 queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(vulkan.physical_device, &queue_count, NULL);
	assert(queue_count > 0);

	VkQueueFamilyProperties* queue_props = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(vulkan.physical_device, &queue_count, queue_props);

	uint64 graphics_queue_index = 0;
	for (graphics_queue_index = 0; graphics_queue_index < queue_count; graphics_queue_index++)
	{
		if (queue_props[graphics_queue_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			break;
	}

	assert(graphics_queue_index < queue_count);

	float queue_priorities[] = { 1.0f };
	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = graphics_queue_index;
	queue_create_info.queueCount = 2;
	assert(queue_props[graphics_queue_index].queueCount >= 2);
	queue_create_info.pQueuePriorities = queue_priorities;

	// Create device
	enabled_extensions[0] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.enabledExtensionCount = 1;
	device_create_info.ppEnabledExtensionNames = enabled_extensions;
	if (enable_validation)
	{
		device_create_info.enabledLayerCount = 1;
		device_create_info.ppEnabledLayerNames = enabled_validation_layers;
	}

	err = vkCreateDevice(vulkan.physical_device, &device_create_info, NULL, &vulkan.device);
	if (err)
	{
		tdDisplayError("vkCreateDevice", err);
		return err;
	}

	vkGetPhysicalDeviceProperties(vulkan.physical_device, &vulkan.device_properties);
	vkGetPhysicalDeviceMemoryProperties(vulkan.physical_device, &vulkan.device_memory_properties);
	vkGetDeviceQueue(vulkan.device, graphics_queue_index, 0, &vulkan.queue);
	vkGetDeviceQueue(vulkan.device, graphics_queue_index, 1, &vulkan.queue2);

	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	VkFormat depth_formats[] =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depth_formats)
	{
		VkFormatProperties format_props;
		vkGetPhysicalDeviceFormatProperties(vulkan.physical_device, format, &format_props);
		// Format must support depth stencil attachment for optimal tiling
		if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			vulkan.depth_format = format;
			break;
		}
	}

	assert(vulkan.depth_format != VK_FORMAT_UNDEFINED);

	VkSemaphoreCreateInfo semaphore_create_info = {};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	err = vkCreateSemaphore(vulkan.device, &semaphore_create_info, NULL, &vulkan.semaphore_present_complete);
	if (err)
	{
		tdDisplayError("vkCreateSemaphore", err);
		return err;
	}

	err = vkCreateSemaphore(vulkan.device, &semaphore_create_info, NULL, &vulkan.semaphore_render_complete);
	if (err)
	{
		tdDisplayError("vkCreateSemaphore", err);
		return err;
	}

	// Swap Chain
	#define GET_INSTANCE_PROC_ADDR(entrypoint)								\
	{                                                                       \
		vulkan.fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(vulkan.instance, "vk"#entrypoint); \
		if (vulkan.fp##entrypoint == NULL)                                         \
		{																    \
			err = VK_ERROR_INITIALIZATION_FAILED;							\
			tdDisplayError("GET_INSTANCE_PROC_ADDR", err);					\
			return err;														\
		}                                                                   \
	}

	#define GET_DEVICE_PROC_ADDR(entrypoint)								\
	{                                                                       \
		vulkan.fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(vulkan.device, "vk"#entrypoint);   \
		if (vulkan.fp##entrypoint == NULL)                                         \
		{																    \
			err = VK_ERROR_INITIALIZATION_FAILED;							\
			tdDisplayError("GET_INSTANCE_PROC_ADDR", err);					\
			return err;														\
		}                                                                   \
	}

	GET_INSTANCE_PROC_ADDR(GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(GetPhysicalDeviceSurfacePresentModesKHR);
	GET_DEVICE_PROC_ADDR(CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(QueuePresentKHR);

	VkWin32SurfaceCreateInfoKHR surface_create_info = {};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hinstance = hinst;
	surface_create_info.hwnd = hwnd;

	err = vkCreateWin32SurfaceKHR(vulkan.instance, &surface_create_info, NULL, &vulkan.surface);
	if (err)
	{
		tdDisplayError("vkCreateWin32SurfaceKHR", err);
		return err;
	}

	VkBool32* supports_present = (VkBool32*)malloc(sizeof(VkBool32) * queue_count);
	for (u64 i = 0; i < queue_count; ++i)
	{
		vulkan.fpGetPhysicalDeviceSurfaceSupportKHR(vulkan.physical_device, i, vulkan.surface, supports_present + i);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32 graphics_queue_node_index = UINT32_MAX;
	uint32 present_queue_node_index = UINT32_MAX;
	for (u64 i = 0; i < queue_count; ++i)
	{
		if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphics_queue_node_index == UINT32_MAX)
				graphics_queue_node_index = i;

			if (supports_present[i] == VK_TRUE)
			{
				graphics_queue_node_index = i;
				present_queue_node_index = i;
				break;
			}
		}
	}

	if (present_queue_node_index == UINT32_MAX)
	{
		// If there's no queue that supports both present and graphics
		// try to find a separate present queue
		for (u64 i = 0; i < queue_count; ++i)
		{
			if (supports_present[i] == VK_TRUE)
			{
				present_queue_node_index = i;
				break;
			}
		}
	}

	// Exit if either a graphics or a presenting queue hasn't been found
	if (graphics_queue_node_index == UINT32_MAX || present_queue_node_index == UINT32_MAX)
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("Could not find a graphics and/or presenting queue", err);
		return err;
	}

	// todo : Add support for separate graphics and presenting queue
	if (graphics_queue_node_index != present_queue_node_index)
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("Separate graphics and presenting queues are not supported yet", err);
		return err;
	}

	vulkan.graphics_queue_node_index = graphics_queue_node_index;

	// Get list of supported surface formats
	uint32 format_count;
	err = vulkan.fpGetPhysicalDeviceSurfaceFormatsKHR(vulkan.physical_device, vulkan.surface, &format_count, NULL);
	if (err)
	{
		tdDisplayError("fpGetPhysicalDeviceSurfaceFormatsKHR1", err);
		return err;
	}
	assert(format_count > 0);

	VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * format_count);
	err = vulkan.fpGetPhysicalDeviceSurfaceFormatsKHR(vulkan.physical_device, vulkan.surface, &format_count, surface_formats);
	if (err)
	{
		tdDisplayError("fpGetPhysicalDeviceSurfaceFormatsKHR2", err);
		return err;
	}

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((format_count == 1) && (surface_formats[0].format == VK_FORMAT_UNDEFINED))
	{
		vulkan.color_format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		// Always select the first available color format
		// If you need a specific format (e.g. SRGB) you'd need to
		// iterate over the list of available surface format and
		// check for it's presence
		vulkan.color_format = surface_formats[0].format;
	}
	vulkan.color_space = surface_formats[0].colorSpace;

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = vulkan.graphics_queue_node_index;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	err = vkCreateCommandPool(vulkan.device, &cmd_pool_info, NULL, &vulkan.command_pool);
	if (err)
	{
		tdDisplayError("vkCreateCommandPool", err);
		return err;
	}

	VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = vulkan.command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;

	err = vkAllocateCommandBuffers(vulkan.device, &command_buffer_allocate_info, &vulkan.setup_command_buffer);
	if (err)
	{
		tdDisplayError("vkAllocateCommandBuffers", err);
		return err;
	}

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	err = vkBeginCommandBuffer(vulkan.setup_command_buffer, &begin_info);
	if (err)
	{
		tdDisplayError("vkBeginCommandBuffer(Setup)", err);
		return err;
	}

	err = InitSwapChain(vulkan, &vulkan.surface_width, &vulkan.surface_height);
	if (err)
	{
		return err;
	}

	for (u64 i = 0; i < vulkan.frame_count; ++i)
	{
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool = vulkan.command_pool;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;

		err = vkAllocateCommandBuffers(vulkan.device, &command_buffer_allocate_info, &vulkan.frames[i].draw_command_buffer);
		if (err)
		{
			tdDisplayError("vkAllocateCommandBuffers(draw)", err);
			return err;
		}
	}

	// Command buffers for submitting present barriers
	err = vkAllocateCommandBuffers(vulkan.device, &command_buffer_allocate_info, &vulkan.pre_present_command_buffer);
	if (err)
	{
		tdDisplayError("vkAllocateCommandBuffers(pre)", err);
		return err;
	}

	err = vkAllocateCommandBuffers(vulkan.device, &command_buffer_allocate_info, &vulkan.post_present_command_buffer);
	if (err)
	{
		tdDisplayError("vkAllocateCommandBuffers(post)", err);
		return err;
	}

	// Create Depth Stencil
	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = vulkan.depth_format;
	image_create_info.extent = { vulkan.surface_width, vulkan.surface_height, 1 };
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	VkImageViewCreateInfo depth_stencil_view_info = {};
	depth_stencil_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depth_stencil_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depth_stencil_view_info.format = vulkan.depth_format;
	depth_stencil_view_info.subresourceRange = {};
	depth_stencil_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depth_stencil_view_info.subresourceRange.baseMipLevel = 0;
	depth_stencil_view_info.subresourceRange.levelCount = 1;
	depth_stencil_view_info.subresourceRange.baseArrayLayer = 0;
	depth_stencil_view_info.subresourceRange.layerCount = 1;

	err = vkCreateImage(vulkan.device, &image_create_info, NULL, &vulkan.depth_stencil_image);
	if (err)
	{
		tdDisplayError("vkCreateImage(depth)", err);
		return err;
	}

	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(vulkan.device, vulkan.depth_stencil_image, &mem_reqs);

	VkMemoryAllocateInfo mem_alloc_info = {};
	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc_info.allocationSize = mem_reqs.size;

	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc_info.memoryTypeIndex))
	{
		tdDisplayError("GetMemoryType(depth)", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc_info, NULL, &vulkan.depth_stencil_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory(depth)", err);
		return err;
	}

	err = vkBindImageMemory(vulkan.device, vulkan.depth_stencil_image, vulkan.depth_stencil_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindImageMemory(depth)", err);
		return err;
	}

	VkImageSubresourceRange subresource_range = {};
	subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	subresource_range.baseMipLevel = 0;
	subresource_range.levelCount = 1;
	subresource_range.layerCount = 1;

	tdVkSetImageLayout(vulkan.setup_command_buffer, vulkan.depth_stencil_image, subresource_range.aspectMask, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, subresource_range);

	depth_stencil_view_info.image = vulkan.depth_stencil_image;
	err = vkCreateImageView(vulkan.device, &depth_stencil_view_info, NULL, &vulkan.depth_stencil_view);
	if (err)
	{
		tdDisplayError("vkBindImageMemory(depth)", err);
		return err;
	}

	VkAttachmentDescription attachments[2];
	attachments[0].format = vulkan.color_format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	attachments[1].format = vulkan.depth_format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference color_reference = {};
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pDepthStencilAttachment = &depth_reference;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 2;
	render_pass_info.pAttachments = attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	err = vkCreateRenderPass(vulkan.device, &render_pass_info, NULL, &vulkan.render_pass);
	if (err)
	{
		tdDisplayError("vkCreateRenderPass", err);
		return err;
	}

	VkPipelineCacheCreateInfo pipeline_cache_info = {};
	pipeline_cache_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	err = vkCreatePipelineCache(vulkan.device, &pipeline_cache_info, NULL, &vulkan.pipeline_cache);
	if (err)
	{
		tdDisplayError("vkCreatePipelineCache", err);
		return err;
	}

	// Frame buffers
	VkImageView image_attachments[2];
	image_attachments[1] = vulkan.depth_stencil_view;

	VkFramebufferCreateInfo frame_buffer_info = {};
	frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frame_buffer_info.renderPass = vulkan.render_pass;
	frame_buffer_info.attachmentCount = 2;
	frame_buffer_info.pAttachments = image_attachments;
	frame_buffer_info.width = vulkan.surface_width;
	frame_buffer_info.height = vulkan.surface_height;
	frame_buffer_info.layers = 1;

	// Create frame buffers for every swap chain image
	for (u64 i = 0; i < vulkan.frame_count; ++i)
	{
		VkFence fence = tdVkCreateFence(vulkan, VK_FENCE_CREATE_SIGNALED_BIT);
		assert(vkGetFenceStatus(vulkan.device, fence) == VK_SUCCESS);
		vulkan.frames[i].fence = fence;

		image_attachments[0] = vulkan.frames[i].image_view;

		err = vkCreateFramebuffer(vulkan.device, &frame_buffer_info, NULL, &vulkan.frames[i].frame_buffer);
		if (err)
		{
			tdDisplayError("vkCreateFramebuffer", err);
			return err;
		}

		vulkan.frames[i].free_resource_mutex = CreateMutex(0, false, 0);

	}

	err = vkEndCommandBuffer(vulkan.setup_command_buffer);
	if (err)
	{
		tdDisplayError("vkEndCommandBuffer", err);
		return err;
	}

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &vulkan.setup_command_buffer;

	err = vkQueueSubmit(vulkan.queue, 1, &submit_info, VK_NULL_HANDLE);
	if (err)
	{
		tdDisplayError("vkQueueSubmit", err);
		return err;
	}

	err = vkQueueWaitIdle(vulkan.queue);
	if (err)
	{
		tdDisplayError("vkQueueWaitIdle", err);
		return err;
	}

	vkFreeCommandBuffers(vulkan.device, vulkan.command_pool, 1, &vulkan.setup_command_buffer);
	vulkan.setup_command_buffer = VK_NULL_HANDLE;

	free(physical_devices);
	free(queue_props);
	free(supports_present);
	free(surface_formats);

	vulkan.is_initialized = true;
	return VK_SUCCESS;
}

}