#include "TdVkTexture.h"
#include <memory>
#include <gli/gli.hpp>

namespace eng {

void tdVkLoadTexture(TdVkInstance& vulkan, const char* filename, VkFormat format, TdVkTexture& texture, bool linear)
{
	VkResult err;
	gli::texture2D tex2D(gli::load(filename));
	assert(!tex2D.empty());

	texture.width = (uint32)tex2D[0].dimensions().x;
	texture.height = (uint32)tex2D[0].dimensions().y;
	texture.mip_levels = tex2D.levels();

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(vulkan.physical_device, format, &format_properties);

	// Only use linear tiling if requested (and supported by the device)
	// Support for linear tiling is mostly limited, so prefer to use optimal tiling instead
	// On most implementations linear tiling will only support a very
	// limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
	VkBool32 use_staging = !linear;

	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = format;
	image_create_info.extent = { (uint32)texture.width, (uint32)texture.height, (uint32)1 };
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
	image_create_info.usage = (use_staging) ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : VK_IMAGE_USAGE_SAMPLED_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	VkCommandBufferAllocateInfo command_alloc_info = {};
	command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_alloc_info.commandPool = vulkan.command_pool;
	command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_alloc_info.commandBufferCount = 1;
	VkCommandBuffer command_buffer;
	err = vkAllocateCommandBuffers(vulkan.device, &command_alloc_info, &command_buffer);
	if (err)
	{
		tdDisplayError("vkAllocateCommandBuffers", err);
		return;
	}

	VkCommandBufferBeginInfo command_begin_info = {};
	command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	err = vkBeginCommandBuffer(command_buffer, &command_begin_info);
	if (err)
	{
		tdDisplayError("vkBeginCommandBuffer", err);
		return;
	}

	VkMemoryRequirements mem_reqs = {};
	VkMemoryAllocateInfo mem_alloc_info = {};
	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	if (use_staging)
	{
		//// Load all available mip levels into linear textures
		//// and copy to optimal tiling target
		struct MipLevel
		{
			VkImage image;
			VkDeviceMemory gpu_mem;
		};

		uint32 size = sizeof(MipLevel) * texture.mip_levels;
		MipLevel* mip_levels = (MipLevel*)malloc(size);
		memset(mip_levels, 0, size);

		for (u64 level = 0; level < texture.mip_levels; ++level)
		{
			image_create_info.extent.width = tex2D[level].dimensions().x;
			image_create_info.extent.height = tex2D[level].dimensions().y;
			image_create_info.extent.depth = 1;

			err = vkCreateImage(vulkan.device, &image_create_info, NULL, &mip_levels[level].image);
			if (err)
			{
				tdDisplayError("vkCreateImage", err);
				return;
			}

			vkGetImageMemoryRequirements(vulkan.device, mip_levels[level].image, &mem_reqs);
			mem_alloc_info.allocationSize = mem_reqs.size;

			if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc_info.memoryTypeIndex))
			{
				err = VK_ERROR_INITIALIZATION_FAILED;
				tdDisplayError("tdVkGetMemoryType", err);
				return;
			}

			err = vkAllocateMemory(vulkan.device, &mem_alloc_info, NULL, &mip_levels[level].gpu_mem);
			if (err)
			{
				tdDisplayError("vkAllocateMemory", err);
				return;
			}

			err = vkBindImageMemory(vulkan.device, mip_levels[level].image, mip_levels[level].gpu_mem, 0);
			if (err)
			{
				tdDisplayError("vkBindImageMemory", err);
				return;
			}

			VkImageSubresource sub_res = {};
			sub_res.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			void *cpu_mem;
			VkSubresourceLayout sub_res_layout = {};
			vkGetImageSubresourceLayout(vulkan.device, mip_levels[level].image, &sub_res, &sub_res_layout);

			err = vkMapMemory(vulkan.device, mip_levels[level].gpu_mem, 0, mem_reqs.size, 0, &cpu_mem);
			if (err)
			{
				tdDisplayError("vkMapMemory", err);
				return;
			}

			memcpy(cpu_mem, tex2D[level].data(), tex2D[level].size());
			vkUnmapMemory(vulkan.device, mip_levels[level].gpu_mem);

			VkImageSubresourceRange sub_res_range = {};
			sub_res_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			sub_res_range.baseMipLevel = 0;
			sub_res_range.levelCount = 1;
			sub_res_range.layerCount = 1;

			tdVkSetImageLayout(command_buffer, mip_levels[level].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sub_res_range);
		}

		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_create_info.mipLevels = texture.mip_levels;
		image_create_info.extent = { (uint32)texture.width, (uint32)texture.height, (uint32)1 };

		err = vkCreateImage(vulkan.device, &image_create_info, NULL, &texture.image);
		if (err)
		{
			tdDisplayError("vkCreateImage", err);
			return;
		}

		vkGetImageMemoryRequirements(vulkan.device, texture.image, &mem_reqs);
		mem_alloc_info.allocationSize = mem_reqs.size;
		if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc_info.memoryTypeIndex))
		{
			err = VK_ERROR_INITIALIZATION_FAILED;
			tdDisplayError("vkCreateBuffer", err);
			return;
		}

		err = vkAllocateMemory(vulkan.device, &mem_alloc_info, NULL, &texture.gpu_mem);
		if (err)
		{
			tdDisplayError("vkAllocateMemory", err);
			return;
		}

		err = vkBindImageMemory(vulkan.device, texture.image, texture.gpu_mem, 0);
		if (err)
		{
			tdDisplayError("vkBindImageMemory", err);
			return;
		}

		VkImageSubresourceRange sub_res_range = {};
		sub_res_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_res_range.baseMipLevel = 0;
		sub_res_range.levelCount = texture.mip_levels;
		sub_res_range.layerCount = 1;

		tdVkSetImageLayout(command_buffer, texture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, sub_res_range);

		// Copy mip levels one by one
		for (u64 level = 0; level < texture.mip_levels; ++level)
		{
			VkImageCopy copy_region = {};
			copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.srcSubresource.baseArrayLayer = 0;
			copy_region.srcSubresource.mipLevel = 0;
			copy_region.srcSubresource.layerCount = 1;
			copy_region.srcOffset = { 0, 0, 0 };
			copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_region.dstSubresource.baseArrayLayer = 0;
			copy_region.dstSubresource.mipLevel = level;
			copy_region.dstSubresource.layerCount = 1;
			copy_region.dstOffset = { 0, 0, 0 };
			copy_region.extent.width = tex2D[level].dimensions().x;
			copy_region.extent.height = tex2D[level].dimensions().y;
			copy_region.extent.depth = 1;

			vkCmdCopyImage(command_buffer, mip_levels[level].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
		}

		// Change texture image layout to shader read for all mip levels after the copy
		texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		tdVkSetImageLayout(command_buffer, texture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.image_layout, sub_res_range);

		// Submit command buffer containing copy and image layout commands
		err = vkEndCommandBuffer(command_buffer);
		if (err)
		{
			tdDisplayError("vkEndCommandBuffer", err);
			return;
		}

		// Create a fence to make sure that the copies have finished before continuing
		VkFence copy_fence;
		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FLAGS_NONE;

		err = vkCreateFence(vulkan.device, &fence_info, NULL, &copy_fence);
		if (err)
		{
			tdDisplayError("vkCreateFence", err);
			return;
		}

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		err = vkQueueSubmit(vulkan.queue, 1, &submit_info, copy_fence);
		if (err)
		{
			tdDisplayError("vkQueueSubmit", err);
			return;
		}

		err = vkWaitForFences(vulkan.device, 1, &copy_fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
		if (err)
		{
			tdDisplayError("vkWaitForFences", err);
			return;
		}

		vkDestroyFence(vulkan.device, copy_fence, NULL);

		for (u64 i = 0; i < texture.mip_levels; ++i)
		{
			vkDestroyImage(vulkan.device, mip_levels[i].image, NULL);
			vkFreeMemory(vulkan.device, mip_levels[i].gpu_mem, NULL);
		}
	}
	else
	{
		// Prefer using optimal tiling, as linear tiling
		// may support only a small set of features
		// depending on implementation (e.g. no mip maps, only one layer, etc.)

		// Check if this support is supported for linear tiling
		assert(format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

		VkImage gpu_image;
		VkDeviceMemory gpu_mem;

		// Load mip map level 0 to linear tiling image
		err = vkCreateImage(vulkan.device, &image_create_info, NULL, &gpu_image);
		vkGetImageMemoryRequirements(vulkan.device, gpu_image, &mem_reqs);
		mem_alloc_info.allocationSize = mem_reqs.size;
		if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc_info.memoryTypeIndex))
		{
			err = VK_ERROR_FORMAT_NOT_SUPPORTED;
			tdDisplayError("tdVkGetMemoryType", err);
			return;
		}

		err = vkAllocateMemory(vulkan.device, &mem_alloc_info, NULL, &gpu_mem);
		if (err)
		{
			tdDisplayError("vkAllocateMemory", err);
			return;
		}

		err = vkBindImageMemory(vulkan.device, gpu_image, gpu_mem, 0);
		if (err)
		{
			tdDisplayError("vkBindImageMemory", err);
			return;
		}

		// Get sub resource layout
		// Mip map count, array layer, etc.
		VkImageSubresource sub_res = {};
		sub_res.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkSubresourceLayout sub_res_layout;
		void* cpu_mem;
		vkGetImageSubresourceLayout(vulkan.device, gpu_image, &sub_res, &sub_res_layout);
		err = vkMapMemory(vulkan.device, gpu_mem, 0, mem_reqs.size, 0, &cpu_mem);
		if (err)
		{
			tdDisplayError("vkMapMemory", err);
			return;
		}

		memcpy(cpu_mem, tex2D[sub_res.mipLevel].data(), tex2D[sub_res.mipLevel].size());
		vkUnmapMemory(vulkan.device, gpu_mem);

		texture.image = gpu_image;
		texture.gpu_mem = gpu_mem;
		texture.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = sub_res.aspectMask;
		subresource_range.levelCount = 1;
		subresource_range.layerCount = 1;
		tdVkSetImageLayout(command_buffer, texture.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, texture.image_layout, subresource_range);

		err = vkEndCommandBuffer(command_buffer);
		if (err)
		{
			tdDisplayError("vkEndCommandBuffer", err);
			return;
		}

		VkFence null_fence = { VK_NULL_HANDLE };
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 0;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		err = vkQueueSubmit(vulkan.queue, 1, &submit_info, null_fence);
		if (err)
		{
			tdDisplayError("vkQueueSubmit", err);
			return;
		}

		err = vkQueueWaitIdle(vulkan.queue);
		if (err)
		{
			tdDisplayError("vkQueueWaitIdle", err);
			return;
		}
	}

	// Create sampler
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 0.0f;//(use_staging) ? (float)texture.mip_levels : 0.0f;
	sampler.maxAnisotropy = 8;
	sampler.anisotropyEnable = VK_FALSE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	err = vkCreateSampler(vulkan.device, &sampler, NULL, &texture.sampler);
	if (err)
	{
		tdDisplayError("vkCreateSampler", err);
		return;
	}

	VkImageViewCreateInfo view = {};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = 1;//(use_staging) ? texture.mip_levels : 1;
	view.image = texture.image;
	err = vkCreateImageView(vulkan.device, &view, NULL, &texture.image_view);
	if (err)
	{
		tdDisplayError("vkCreateImageView", err);
		return;
	}
}

TdVkTexture* tdVkLoadTexture(TdVkInstance& vulkan, const char* filename, VkFormat format, bool linear)
{
	TdVkTexture* texture = (TdVkTexture*)malloc(sizeof(TdVkTexture));
	memset(texture, 0, sizeof(texture));
	tdVkLoadTexture(vulkan, filename, format, *texture, linear);
	return texture;
}

};
