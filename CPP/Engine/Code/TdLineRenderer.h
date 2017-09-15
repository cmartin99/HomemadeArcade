#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdLineRenderer
{
	enum AddLineFlagBits
	{
		TD_ADDLINE_USAGE_GPU_BUFFER_BIT		= 0x00000001,
		TD_ADDLINE_USAGE_HOST_BUFFER_BIT	= 0x00000002,
		TD_ADDLINE_USAGE_CUBE_OUTLINE_BIT	= 0x00000004,
		TD_ADDLINE_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};

	struct VertexLine
	{
		Vector4 pos;
		Color color;
	};

	struct LineData
	{
		uint16 id;
		Vector3 start;
		Vector3 end;
		Color color;
	};

	struct LineStorage
	{
		bool is_dirty;
		TdVkBuffer vb;
		TdArray1<LineData> lines;
	};

	struct
	{
		Matrix view_proj;
		Vector3 cam_pos;
		float far_clip;
	} ubo_cpu;

	TdVkInstance* vulkan;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	TdArray1<VkShaderModule> shader_modules;
	TdArray<VertexLine> vertices;
	TdVkBuffer ubo;
	LineStorage lines_cpu;
	LineStorage lines_gpu;
};

uint16 tdVkAddLine(TdLineRenderer&, uint32 flags, Vector3 start, Vector3 end, Color);
void tdVkDeleteLine(TdLineRenderer&, uint16 id, uint32 flags);
void tdVkClearLines(TdLineRenderer&, uint32 flags);
void tdVkLineRendererPresent(TdLineRenderer&, VkCommandBuffer, const Matrix& view_proj, Vector3 cam_pos, float far_clip);
VkResult tdVkLoadContent(TdLineRenderer&, TdVkInstance&, uint32 max_vertex_count, uint32 line_width);

}