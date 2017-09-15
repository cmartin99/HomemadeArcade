#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdLineRenderer2D
{
	enum AddLineFlagBits
	{
		TD_ADDLINE_USAGE_GPU_BUFFER_BIT		= 0x00000001,
		TD_ADDLINE_USAGE_HOST_BUFFER_BIT	= 0x00000002,
		TD_ADDLINE_USAGE_BOX_OUTLINE_BIT	= 0x00000004,
		TD_ADDLINE_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};

	struct VertexLine
	{
		Vector2 pos;
		Color color;
	};

	struct LineData
	{
		uint16 id;
		Vector2 start;
		Vector2 end;
		Color color;
	};

	struct LineStorage
	{
		bool is_dirty;
		TdVkBuffer vb;
		TdArray<LineData> lines;
	};

	TdVkInstance* vulkan;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	TdArray<VkShaderModule> shader_modules;
	TdArray<VertexLine> vertices;
	LineStorage lines_cpu;
	LineStorage lines_gpu;
};

uint16 tdVkAddLine(TdLineRenderer2D*, uint32 flags, Vector2 start, Vector2 end, Color);
void tdVkDeleteLine(TdLineRenderer2D*, uint16 id, uint32 flags);
void tdVkClearLines(TdLineRenderer2D*, uint32 flags);
void tdVkLineRendererPresent(TdLineRenderer2D*, VkCommandBuffer);
VkResult tdVkLoadContent(TdLineRenderer2D*, TdVkInstance*, uint32 max_vertex_count, uint32 line_width);

}