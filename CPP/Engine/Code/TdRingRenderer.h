#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdRingRenderer
{
	struct RingData
	{
		uint16 id;
		uint16 segs;
		Vector3 pos;
		Vector3 rot;
		float radius;
		float width;
		Color color;
	};

	struct VertexRing
	{
		Vector4 pos;
		Color color;
	};

	struct
	{
		Matrix view_proj;
	} ubo_cpu;

	TdVkInstance& vulkan;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	TdArray1<VkShaderModule> shader_modules;
	TdArray1<RingData> rings;
	TdArray1<VertexRing> vertices;
	TdVkBuffer ubo;
	TdVkBuffer vb;
	bool vb_dirty;

	TdRingRenderer(TdVkInstance& vulkan) : vulkan(vulkan), vertices(10000) {}
};

void tdVkAddRing(TdRingRenderer&, uint16 id, Vector3 pos, Vector3 rot, float radius, float width, uint16 segs, Color);
void tdVkDeleteRing(TdRingRenderer&, uint16 id);
void tdVkRingRendererPresent(TdRingRenderer&, VkCommandBuffer, const Matrix& view_proj);
void tdVkRingRendererClear(TdRingRenderer&);
VkResult tdVkLoadContent(TdRingRenderer&);

}