#pragma once
#include "TdDataTypes.h"

#define VK_FLAGS_NONE 0

namespace eng {

struct TdVkBuffer
{
	uint32 count;
	uint32 data_size;
	VkBuffer buffer;
	VkDeviceMemory gpu_mem;
	void* cpu_mem;
	const char* name;
};

struct TdVkInstance
{
	struct FrameData
	{
		VkImage image;
		VkImageView image_view;
		VkFramebuffer frame_buffer;
		VkCommandBuffer draw_command_buffer;
		TdArray1<TdVkBuffer> buffer_release;
		TdArray1<TdVkBuffer> buffer_release_log;
		TdMutexHandle free_resource_mutex;
		VkFence fence;
	};

	bool is_initialized = false;
	bool enable_validation = false;
	VkInstance instance;
	VkDevice device;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceMemoryProperties device_memory_properties;
	VkQueue queue;
	//VkQueue queue2;
	VkFormat depth_format;
	VkCommandPool command_pool;
	VkCommandBuffer setup_command_buffer = VK_NULL_HANDLE;
	VkCommandBuffer pre_present_command_buffer = VK_NULL_HANDLE;
	VkCommandBuffer post_present_command_buffer = VK_NULL_HANDLE;
	TdArray1<VkCommandBuffer> command_buffer_free;
	TdArray1<VkCommandBuffer> command_buffer_free2;
	VkRenderPass render_pass;
	VkPipelineCache pipeline_cache;
	uint32 frame_count;
	uint32 current_frame = 0;
	FrameData* frames;
	VkImage depth_stencil_image;
	VkImageView depth_stencil_view;
	VkDeviceMemory depth_stencil_mem;
	VkSemaphore semaphore_present_complete;
	VkSemaphore semaphore_render_complete;
	TdVkBuffer ib_tri;
	TdVkBuffer ib_line;

	VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
	VkSurfaceKHR surface;
	uint32 surface_width;
	uint32 surface_height;
	uint32 graphics_queue_node_index = UINT32_MAX;
	VkFormat color_format;
	VkColorSpaceKHR color_space;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
};

void tdVkBindAndMapBuffer(TdVkInstance*, TdVkBuffer&, uint32 stride, uint32 count);
void tdVkAllocateBuffer(TdVkInstance *, TdVkBuffer &buffer, uint32 stride, uint32 count, VkBufferUsageFlagBits usage);
ALWAYS_INLINE void tdVkAllocateBindAndMapBuffer(TdVkInstance *vulkan, TdVkBuffer &buffer, uint32 stride, uint32 count, VkBufferUsageFlagBits usage) { tdVkAllocateBuffer(vulkan, buffer, stride, count, usage); tdVkBindAndMapBuffer(vulkan, buffer, stride, count); }
ALWAYS_INLINE void tdVkAllocateVertexBuffer(TdVkInstance* vulkan, TdVkBuffer& buffer, uint32 stride, uint32 count) { tdVkAllocateBuffer(vulkan, buffer, stride, count, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT); }
ALWAYS_INLINE void tdVkAllocateIndexBuffer(TdVkInstance* vulkan, TdVkBuffer& buffer, uint32 count) { tdVkAllocateBuffer(vulkan, buffer, sizeof(uint32), count, VK_BUFFER_USAGE_INDEX_BUFFER_BIT); }
ALWAYS_INLINE void tdVkAllocateBindAndMapVertexBuffer(TdVkInstance *vulkan, TdVkBuffer &buffer, uint32 stride, uint32 count) { tdVkAllocateBindAndMapBuffer(vulkan, buffer, stride, count, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT); tdVkBindAndMapBuffer(vulkan, buffer, stride, count); }
ALWAYS_INLINE void tdVkAllocateBindAndMapIndexBuffer(TdVkInstance *vulkan, TdVkBuffer &buffer, uint32 count) { tdVkAllocateBindAndMapBuffer(vulkan, buffer, sizeof(uint32), count, VK_BUFFER_USAGE_INDEX_BUFFER_BIT); tdVkBindAndMapBuffer(vulkan, buffer, sizeof(uint32), count); }
VkEvent tdVkCreateEvent(TdVkInstance*);
VkFence tdVkCreateFence(TdVkInstance*, VkFenceCreateFlags);
VkResult tdVkWaitForFence(TdVkInstance*, VkFence);
VkResult tdVkWaitForFrameFence(TdVkInstance*);
void tdVkBindIndexBufferTri(TdVkInstance*, VkCommandBuffer, uint32 index_count);
void tdVkBindIndexBufferLine(TdVkInstance*, VkCommandBuffer, uint32 index_count);
VkResult tdVkUpdateMappableBuffer(VkCommandBuffer, TdVkBuffer, VkDeviceSize offset, VkDeviceSize size);
VkResult tdVkUpdateMappableBuffer(VkDevice, TdVkBuffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags);
VkBool32 tdVkGetMemoryType(TdVkInstance*, uint32 type_bits, VkFlags, uint32 * type_index);
void tdVkSetImageLayout(VkCommandBuffer, VkImage, VkImageAspectFlags, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange);
void tdVkFreeResource(TdVkInstance*, VkBuffer, VkDeviceMemory, const char *name);
void tdVkFreeResource(TdVkInstance*, TdVkBuffer, const char *name);
void tdVkUpdateFreeResources(TdVkInstance*);
void tdVkFreeCommandBuffer(TdVkInstance*, VkQueue, VkCommandBuffer);
void tdVkUpdateFreeCommandBuffers(TdVkInstance*, VkQueue);
VkPipelineShaderStageCreateInfo tdVkLoadShader(TdVkInstance*, const char *file_name, VkShaderStageFlagBits stage);
VkResult tdVkInitVulkan(TdVkInstance*, const char *name, bool enable_validation, HINSTANCE hinst, HWND hwnd);

}