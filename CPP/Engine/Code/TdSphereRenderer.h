#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdSphereRenderer
{
	enum AddSphereFlagBits
	{
		TD_ADDSPHERE_USAGE_GPU_BUFFER_BIT = 0x00000000,
		TD_ADDSPHERE_USAGE_HOST_BUFFER_BIT = 0x00000001,
		TD_ADDSPHERE_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};

	struct VertexSphere
	{
		Vector4 pos;
		Color color;
	};

	struct SphereData
	{
		uint16 id;
		uint16 tessellation;
		Vector3 center;
		float radius;
		Color color;
	};

	struct SphereStorage
	{
		bool is_dirty;
		TdVkBuffer vb;
		TdVkBuffer ib;
		TdArray1<SphereData> spheres;
	};

	struct
	{
		Matrix view_proj;
		Vector3 cam_pos;
		float far_clip;
	} ubo_cpu;

	TdVkInstance& vulkan;
	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
	TdArray1<VkShaderModule> shader_modules;
	TdArray1<VertexSphere>* vertices1;
	TdArray1<VertexSphere>* vertices2;
	TdArray1<VertexSphere>* vertices3;
	TdArray1<uint32>* indices1;
	TdArray1<uint32>* indices2;
	TdArray1<uint32>* indices3;
	TdVkBuffer ubo;
	SphereStorage spheres_cpu;
	SphereStorage spheres_gpu;

	TdSphereRenderer(TdVkInstance& vulkan) : vulkan(vulkan), vertices1(NULL), vertices2(NULL), vertices3(NULL), indices1(NULL), indices2(NULL), indices3(NULL) {}
};

uint16 tdVkAddSphere(TdSphereRenderer&, uint32 flags, Vector3 center, float radius, uint16 tessellation, Color);
void tdVkDeleteSphere(TdSphereRenderer&, uint16 id, uint32 flags);
void tdVkClearSpheres(TdSphereRenderer&, uint32 flags);
void tdVkSphereRendererPresent(TdSphereRenderer&, VkCommandBuffer, const Matrix& view_proj, Vector3 cam_pos, float far_clip);
VkResult tdVkLoadContent(TdSphereRenderer&);

}