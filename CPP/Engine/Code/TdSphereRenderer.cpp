#include "TdSphereRenderer.h"

namespace eng {

uint16 tdVkAddSphere(TdSphereRenderer& renderer, uint32 flags, Vector3 center, float radius, uint16 tessellation, Color color)
{
	TdSphereRenderer::SphereStorage& storage = (flags & TdSphereRenderer::TD_ADDSPHERE_USAGE_HOST_BUFFER_BIT)
		? renderer.spheres_cpu : renderer.spheres_gpu;

	TdSphereRenderer::SphereData sphere = { (uint16)(storage.spheres.GetCount() + 1), tessellation, center, radius, color };
	storage.spheres.Add(sphere);
	storage.is_dirty = true;
	return sphere.id;
}

void tdVkDeleteSphere(TdSphereRenderer& renderer, uint16 id, uint32 flags)
{
}

void tdVkClearSpheres(TdSphereRenderer& renderer, uint32 flags)
{
	if (flags & TdSphereRenderer::TD_ADDSPHERE_USAGE_HOST_BUFFER_BIT)
	{
		renderer.spheres_cpu.spheres.Clear();
		renderer.spheres_cpu.is_dirty = true;
	}
	else
	{
		renderer.spheres_gpu.spheres.Clear();
		renderer.spheres_gpu.is_dirty = true;
	}
}

VkResult UpdateStorageCPU(TdSphereRenderer& renderer, VkCommandBuffer& command_buffer, TdSphereRenderer::SphereStorage& storage)
{
	if (storage.vb.count)
	{
		tdVkFreeResource(renderer.vulkan, storage.vb.buffer, storage.vb.gpu_mem, "SphereRenderer::UpdateStorageCPU");
	}

	storage.vb.count = renderer.vertices3 ? renderer.vertices3->GetCount() : 0;
	if (storage.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;

	size_t vb_size = storage.vb.count * sizeof(TdSphereRenderer::VertexSphere);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements mem_reqs;

	VkBufferCreateInfo vb_info = {};
	vb_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vb_info.size = vb_size;
	vb_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	err = vkCreateBuffer(vulkan.device, &vb_info, NULL, &storage.vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, storage.vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &storage.vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan.device, storage.vb.gpu_mem, 0, mem_alloc.allocationSize, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, renderer.vertices3->ptr(), vb_size);
	vkUnmapMemory(vulkan.device, storage.vb.gpu_mem);

	err = vkBindBufferMemory(vulkan.device, storage.vb.buffer, storage.vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	return VK_SUCCESS;
}

VkResult UpdateIBStorageCPU(TdSphereRenderer& renderer, VkCommandBuffer& command_buffer, TdSphereRenderer::SphereStorage& storage)
{
	if (storage.ib.count)
	{
		tdVkFreeResource(renderer.vulkan, storage.ib.buffer, storage.ib.gpu_mem, "SphereRenderer::UpdateIBStorageCPU");
	}

	storage.ib.count = renderer.indices3 ? renderer.indices3->GetCount() : 0;
	if (storage.ib.count < 1) return VK_SUCCESS;

	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;

	size_t ib_size = storage.ib.count * sizeof(uint32);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements mem_reqs;

	VkBufferCreateInfo ib_info = {};
	ib_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ib_info.size = ib_size;
	ib_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &storage.ib.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, storage.ib.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &storage.ib.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan.device, storage.ib.gpu_mem, 0, mem_alloc.allocationSize, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, renderer.indices3->ptr(), ib_size);
	vkUnmapMemory(vulkan.device, storage.ib.gpu_mem);

	err = vkBindBufferMemory(vulkan.device, storage.ib.buffer, storage.ib.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	return VK_SUCCESS;
}

VkResult UpdateStorageGPU(TdSphereRenderer& renderer, VkCommandBuffer& command_buffer, TdSphereRenderer::SphereStorage& storage)
{
	if (storage.vb.count)
	{
		tdVkFreeResource(renderer.vulkan, storage.vb.buffer, storage.vb.gpu_mem, "SphereRenderer::UpdateStorageGPU");
	}

	storage.vb.count = renderer.vertices3 ? renderer.vertices3->GetCount() : 0;
	if (storage.vb.count < 1) return VK_SUCCESS;

	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;

	size_t vb_size = storage.vb.count * sizeof(TdSphereRenderer::VertexSphere);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements mem_reqs;

	TdVkBuffer stage_vb;
	VkBufferCreateInfo vb_info = {};
	vb_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vb_info.size = vb_size;
	vb_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	err = vkCreateBuffer(vulkan.device, &vb_info, NULL, &stage_vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
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

	memcpy(cpu_mem, renderer.vertices3->ptr(), vb_size);
	vkUnmapMemory(vulkan.device, stage_vb.gpu_mem);

	err = vkBindBufferMemory(vulkan.device, stage_vb.buffer, stage_vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	// Create the destination buffer with device only visibility
	vb_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	err = vkCreateBuffer(vulkan.device, &vb_info, NULL, &storage.vb.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, storage.vb.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &storage.vb.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	err = vkBindBufferMemory(vulkan.device, storage.vb.buffer, storage.vb.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	VkBufferCopy copy_region = {};
	copy_region.size = vb_size;
	vkCmdCopyBuffer(command_buffer, stage_vb.buffer, storage.vb.buffer, 1, &copy_region);

	tdVkFreeResource(renderer.vulkan, stage_vb.buffer, stage_vb.gpu_mem, "SphereRenderer::UpdateStorageGPU_Stage");
	return VK_SUCCESS;
}

VkResult UpdateIBStorageGPU(TdSphereRenderer& renderer, VkCommandBuffer& command_buffer, TdSphereRenderer::SphereStorage& storage)
{
	if (storage.ib.count)
	{
		tdVkFreeResource(renderer.vulkan, storage.ib.buffer, storage.ib.gpu_mem, "SphereRenderer::UpdateIBStorageGPU");
	}

	storage.ib.count = renderer.indices3 ? renderer.indices3->GetCount() : 0;
	if (storage.ib.count < 1) return VK_SUCCESS;

	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;

	size_t ib_size = storage.ib.count * sizeof(uint32);
	TdVkBuffer stage_ib;

	// Index buffer
	VkBufferCreateInfo ib_info = {};
	ib_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ib_info.size = ib_size;
	ib_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &stage_ib.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	VkMemoryRequirements mem_reqs = {};
	vkGetBufferMemoryRequirements(vulkan.device, stage_ib.buffer, &mem_reqs);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;

	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &stage_ib.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	void *cpu_mem;
	err = vkMapMemory(vulkan.device, stage_ib.gpu_mem, 0, ib_size, 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return err;
	}

	memcpy(cpu_mem, renderer.indices3->ptr(), ib_size);
	vkUnmapMemory(vulkan.device, stage_ib.gpu_mem);

	err = vkBindBufferMemory(vulkan.device, stage_ib.buffer, stage_ib.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	ib_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	err = vkCreateBuffer(vulkan.device, &ib_info, NULL, &storage.ib.buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return err;
	}

	vkGetBufferMemoryRequirements(vulkan.device, storage.ib.buffer, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("GetMemoryType", err);
		return err;
	}

	err = vkAllocateMemory(vulkan.device, &mem_alloc, NULL, &storage.ib.gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return err;
	}

	err = vkBindBufferMemory(vulkan.device, storage.ib.buffer, storage.ib.gpu_mem, 0);
	if (err)
	{
		tdDisplayError("vkBindBufferMemory", err);
		return err;
	}

	VkBufferCopy copy_region = {};
	copy_region.size = ib_size;
	vkCmdCopyBuffer(command_buffer, stage_ib.buffer, storage.ib.buffer, 1, &copy_region);

	tdVkFreeResource(vulkan, stage_ib.buffer, stage_ib.gpu_mem, "SphereRenderer::UpdateIBStorageGPU_Stage");
	return VK_SUCCESS;
}

void BuildVertices(TdSphereRenderer& renderer, TdArray1<TdSphereRenderer::SphereData>& spheres)
{
	int count = spheres.GetCount();
	if (count < 1) return;

	if (!renderer.vertices1)
	{
		renderer.vertices1 = new TdArray1<TdSphereRenderer::VertexSphere>(10000);
		renderer.vertices2 = new TdArray1<TdSphereRenderer::VertexSphere>(5000);
		renderer.vertices3 = new TdArray1<TdSphereRenderer::VertexSphere>(50000);
		renderer.indices1 = new TdArray1<uint32>(20000);
		renderer.indices2 = new TdArray1<uint32>(10000);
		renderer.indices3 = new TdArray1<uint32>(100000);
	}

	TdArray1<TdSphereRenderer::VertexSphere>* vertices1 = renderer.vertices1;
	TdArray1<TdSphereRenderer::VertexSphere>* vertices2 = renderer.vertices2;
	TdArray1<TdSphereRenderer::VertexSphere>* vertices3 = renderer.vertices3;
	TdArray1<uint32>* indices1 = renderer.indices1;
	TdArray1<uint32>* indices2 = renderer.indices2;
	TdArray1<uint32>* indices3 = renderer.indices3;

	TdSphereRenderer::SphereData* sphere = spheres.ptr();
	TdSphereRenderer::VertexSphere v;
	v.pos.w = 1;

	const static float t = (1.0f + sqrt(5.0f)) / 2.0f;
	const static uint32 init_indices[] =
	{
		0,11,5,0,5,1,0,1,7,0,7,10,0,10,11,
		1,5,9,5,11,4,11,10,2,10,7,6,7,1,8,
		3,9,4,3,4,2,3,2,6,3,6,8,3,8,9,
		4,9,5,2,4,11,6,2,10,8,6,7,9,8,1
	};

	while (count--)
	{
		vertices1->Clear();
		indices1->Clear();

		v.color.r = sphere->color.r;
		v.color.g = sphere->color.g;
		v.color.b = sphere->color.b;
		v.color.a = sphere->color.a;

		v.pos.x = -1;
		v.pos.y = t;
		v.pos.z = 0;
		vertices1->Add(v);
		v.pos.x = 1;
		vertices1->Add(v);
		v.pos.x = -1;
		v.pos.y = -t;
		vertices1->Add(v);
		v.pos.x = 1;
		vertices1->Add(v);

		v.pos.x = 0;
		v.pos.y = -1;
		v.pos.z = t;
		vertices1->Add(v);
		v.pos.y = 1;
		vertices1->Add(v);
		v.pos.y = -1;
		v.pos.z = -t;
		vertices1->Add(v);
		v.pos.y = 1;
		vertices1->Add(v);

		v.pos.x = t;
		v.pos.y = 0;
		v.pos.z = -1;
		vertices1->Add(v);
		v.pos.z = 1;
		vertices1->Add(v);
		v.pos.z = -1;
		v.pos.x = -t;
		vertices1->Add(v);
		v.pos.z = 1;
		vertices1->Add(v);

		indices1->PushCount(60);
		memcpy(indices1->ptr(), init_indices, sizeof(uint32) * 60);

		for (int r = 0; r < sphere->tessellation; r++)
		{
			TdSphereRenderer::VertexSphere* v_refine = vertices1->ptr();
			uint32* i_refine = indices1->ptr();
			int face_count = indices1->GetCount() / 3;
			vertices2->Clear();
			indices2->Clear();

			for (int f = 0; f < face_count; f++)
			{
				int i = vertices2->GetCount();

				Vector3 v1 = v_refine[i_refine[f * 3 + 0]].pos;
				Vector3 v2 = v_refine[i_refine[f * 3 + 1]].pos;
				Vector3 v3 = v_refine[i_refine[f * 3 + 2]].pos;

				Vector3 mid1((v1.x + v2.x) / 2.0f, (v1.y + v2.y) / 2.0f, (v1.z + v2.z) / 2.0f);
				Vector3 mid2((v2.x + v3.x) / 2.0f, (v2.y + v3.y) / 2.0f, (v2.z + v3.z) / 2.0f);
				Vector3 mid3((v3.x + v1.x) / 2.0f, (v3.y + v1.y) / 2.0f, (v3.z + v1.z) / 2.0f);

				v.pos.x = v1.x;
				v.pos.y = v1.y;
				v.pos.z = v1.z;
				vertices2->Add(v);
				v.pos.x = mid1.x;
				v.pos.y = mid1.y;
				v.pos.z = mid1.z;
				vertices2->Add(v);
				v.pos.x = v2.x;
				v.pos.y = v2.y;
				v.pos.z = v2.z;
				vertices2->Add(v);
				v.pos.x = mid2.x;
				v.pos.y = mid2.y;
				v.pos.z = mid2.z;
				vertices2->Add(v);
				v.pos.x = v3.x;
				v.pos.y = v3.y;
				v.pos.z = v3.z;
				vertices2->Add(v);
				v.pos.x = mid3.x;
				v.pos.y = mid3.y;
				v.pos.z = mid3.z;
				vertices2->Add(v);

				indices2->Add(i + 0);
				indices2->Add(i + 1);
				indices2->Add(i + 5);

				indices2->Add(i + 1);
				indices2->Add(i + 2);
				indices2->Add(i + 3);

				indices2->Add(i + 3);
				indices2->Add(i + 4);
				indices2->Add(i + 5);

				indices2->Add(i + 1);
				indices2->Add(i + 3);
				indices2->Add(i + 5);
			}

			TdArray1<TdSphereRenderer::VertexSphere>* vert3 = vertices1;
			vertices1 = vertices2;
			vertices2 = vert3;

			TdArray1<uint32>* ind3 = indices1;
			indices1 = indices2;
			indices2 = ind3;
		}

		TdSphereRenderer::VertexSphere* v_start = vertices1->ptr();
		int vert_count = vertices1->GetCount();
		int vert_count3 = vertices3->GetCount();
		vertices3->PushCount(vert_count);
		TdSphereRenderer::VertexSphere* v3_start = vertices3->ptr(vert_count3);
		Vector3 n1, n2;

		while (vert_count--)
		{
			n1.x = v_start->pos.x;
			n1.y = v_start->pos.y;
			n1.z = v_start->pos.z;
			n2 = normalize(n1);
			v3_start->pos.x = n2.x;
			v3_start->pos.y = n2.y;
			v3_start->pos.z = n2.z;
			v3_start->pos.w = 1;
			v3_start->pos.x *= sphere->radius;
			v3_start->pos.y *= sphere->radius;
			v3_start->pos.z *= sphere->radius;
			v3_start->pos.x += sphere->center.x;
			v3_start->pos.y += sphere->center.y;
			v3_start->pos.z += sphere->center.z;
			v3_start->color.r = sphere->color.r;
			v3_start->color.g = sphere->color.g;
			v3_start->color.b = sphere->color.b;
			v3_start->color.a = sphere->color.a;
			++v_start;
			++v3_start;
		}

		int ind_count = indices1->GetCount();
		uint32* i_start = indices1->ptr();
		int ind_count3 = indices3->GetCount();
		indices3->PushCount(ind_count);
		uint32* i3_start = indices3->ptr(ind_count3);

		while (ind_count--)
		{
			*i3_start = *i_start + vert_count3;
			++i_start;
			++i3_start;
		}

		sphere++;
	}
}

void tdVkSphereRendererPresent(TdSphereRenderer& renderer, VkCommandBuffer command_buffer, const Matrix& view_proj, Vector3 cam_pos, float far_clip)
{
	if (renderer.spheres_cpu.is_dirty)
	{
		BuildVertices(renderer, renderer.spheres_cpu.spheres);
		UpdateStorageCPU(renderer, command_buffer, renderer.spheres_cpu);
		UpdateIBStorageCPU(renderer, command_buffer, renderer.spheres_cpu);
		renderer.spheres_cpu.is_dirty = false;
		if (renderer.vertices1) renderer.vertices1->Clear();
		if (renderer.vertices2) renderer.vertices2->Clear();
		if (renderer.vertices3) renderer.vertices3->Clear();
		if (renderer.indices1) renderer.indices1->Clear();
		if (renderer.indices2) renderer.indices2->Clear();
		if (renderer.indices3) renderer.indices3->Clear();
	}

	if (renderer.spheres_gpu.is_dirty)
	{
		BuildVertices(renderer, renderer.spheres_gpu.spheres);
		UpdateStorageGPU(renderer, command_buffer, renderer.spheres_gpu);
		UpdateIBStorageGPU(renderer, command_buffer, renderer.spheres_gpu);
		renderer.spheres_gpu.is_dirty = false;
		if (renderer.vertices1) renderer.vertices1->Clear();
		if (renderer.vertices2) renderer.vertices2->Clear();
		if (renderer.vertices3) renderer.vertices3->Clear();
		if (renderer.indices1) renderer.indices1->Clear();
		if (renderer.indices2) renderer.indices2->Clear();
		if (renderer.indices3) renderer.indices3->Clear();
	}

	if (renderer.spheres_cpu.vb.count > 0 || renderer.spheres_gpu.vb.count > 0)
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.pipeline_layout, 0, 1, &renderer.descriptor_set, 0, NULL);

		renderer.ubo_cpu.view_proj = view_proj;
		renderer.ubo_cpu.cam_pos = cam_pos;
		renderer.ubo_cpu.far_clip = far_clip;
		tdVkUpdateMappableBuffer(renderer.vulkan.device, renderer.ubo, 0, sizeof(renderer.ubo_cpu), 0, &renderer.ubo_cpu);

		VkDeviceSize offsets[1] = { 0 };

		uint32 vert_count = renderer.spheres_cpu.vb.count;
		if (vert_count)
		{
			vkCmdBindIndexBuffer(command_buffer, renderer.spheres_cpu.ib.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer.spheres_cpu.vb.buffer, offsets);
			vkCmdDrawIndexed(command_buffer, renderer.spheres_cpu.ib.count, 1, 0, 0, 0);
#ifdef _PROFILE_
			++draw_calls;
#endif
		}

		vert_count = renderer.spheres_gpu.vb.count;
		if (vert_count)
		{
			vkCmdBindIndexBuffer(command_buffer, renderer.spheres_gpu.ib.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &renderer.spheres_gpu.vb.buffer, offsets);
			vkCmdDrawIndexed(command_buffer, renderer.spheres_gpu.ib.count, 1, 0, 0, 0);
#ifdef _PROFILE_
			++draw_calls;
#endif
		}
	}
}

VkResult tdVkLoadContent(TdSphereRenderer& renderer)
{
	VkResult err;
	TdVkInstance& vulkan = renderer.vulkan;
	renderer.spheres_cpu.vb.count = 0;
	renderer.spheres_cpu.vb.buffer = VK_NULL_HANDLE;
	renderer.spheres_cpu.vb.gpu_mem = VK_NULL_HANDLE;
	renderer.spheres_cpu.ib.buffer = VK_NULL_HANDLE;
	renderer.spheres_cpu.ib.gpu_mem = VK_NULL_HANDLE;
	renderer.spheres_gpu.vb.count = 0;
	renderer.spheres_gpu.vb.buffer = VK_NULL_HANDLE;
	renderer.spheres_gpu.vb.gpu_mem = VK_NULL_HANDLE;
	renderer.spheres_gpu.ib.buffer = VK_NULL_HANDLE;
	renderer.spheres_gpu.ib.gpu_mem = VK_NULL_HANDLE;

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
	rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterization_state.depthClampEnable = VK_FALSE;
	rasterization_state.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state.depthBiasEnable = VK_FALSE;
	rasterization_state.lineWidth = 1;

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
	shader_stages[0] = tdVkLoadShader(vulkan, "TdSphereRenderer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[1] = tdVkLoadShader(vulkan, "TdSphereRenderer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	renderer.shader_modules.Add(shader_stages[0].module);
	renderer.shader_modules.Add(shader_stages[1].module);

	// Define vertex layout
	VkVertexInputBindingDescription binding_descs = {};
	binding_descs.stride = sizeof(TdSphereRenderer::VertexSphere);
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