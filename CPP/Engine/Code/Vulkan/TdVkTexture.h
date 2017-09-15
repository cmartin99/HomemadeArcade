#pragma once
#include "TdVkInstance.h"

namespace eng {

struct TdVkTexture
{
	int width, height;
	uint32 mip_levels;
	uint32 layer_count;
	VkImage image;
	VkImageView image_view;
	VkImageLayout image_layout;
	VkSampler sampler;
	VkDeviceMemory gpu_mem;
};

TdVkTexture* tdVkLoadTexture(TdVkInstance&, const char* filename, VkFormat, bool force_linear);
void tdVkLoadTexture(TdVkInstance&, const char* filename, VkFormat, TdVkTexture&, bool force_linear);

}