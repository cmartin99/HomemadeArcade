#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdSpriteBatch
{
	const static int MAX_TEXTURES = 16;
	const static int MAX_VERTICES = 100000;

#ifdef _PROFILE_
	struct DebugInfo
	{
		int32 vk_draw_calls;
		int32 td_draw_calls;
		int32 batch_count;
	} debug_info;
#endif

	struct Batch
	{
		int16 tex_id;
		int32 vertex_count;
		int32 vertex_offset;
	};

	struct VertexSprite
	{
		float type;
		float x, y, z;
		float u, v;
		Color color;
	};

	struct UniformData
	{
		VkBuffer buffer;
		VkDeviceMemory gpu_mem;
		VkDescriptorBufferInfo descriptor;
	};

	struct TextureData
	{
		int32 vertex_count;
		TdVkTexture* texture;
	};

	TdVkInstance* vulkan;
	int32 vertex_count;
	int32 max_vertices;
	bool distance_field;
	TdArray<Batch> batches;
	TdArray<TextureData> textures;
	VkBuffer vb_buffer;
	VkDeviceMemory vb_gpu_mem;
	VertexSprite* vb_cpu_mem;
	VkDescriptorSet* descriptor_sets;
	VkDescriptorSet descriptor_set_ubo;
	VkPipeline pso;
	VkDescriptorPool descriptor_pool_image;
	VkDescriptorPool descriptor_pool_ubo;
	VkPipelineLayout pipeline_layout;
	UniformData ubo_info_fs;
	int32 data_size;

	struct
	{
		Vector4 outlineColor = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		float outlineWidth = 0.6f;
		float outline = true;
	} ubo_fs;
};

void tdVkSpriteBatchInit(TdVkInstance*, TdSpriteBatch*, TdMemoryArena* arena, bool distance_field = false, int32 = TdSpriteBatch::MAX_VERTICES, int32 = TdSpriteBatch::MAX_TEXTURES);
void tdVkSpriteBatchPresent(TdSpriteBatch*, VkCommandBuffer);
Vector2 tdVkSpriteBatchGetTextSize(TdSpriteBatch *sb, const char *text, int32 text_len, float scale = 1.0f);

void tdVkDrawBox(TdSpriteBatch*, float x, float y, float w, float h, const Color&, float depth = 1, float rot = 0, const Vector2* origin = nullptr, int32 flip = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* = nullptr);
void tdVkDrawBoxO(TdSpriteBatch*, float x, float y, float w, float h, const Color&, float border_size, const Color& border_color, float depth = 1, float rot = 0, const Vector2* origin = nullptr);
void tdVkDrawBoxX(TdSpriteBatch *, float wt, float wb, float h, const Color &, const Matrix*, float depth = 1);
inline void tdVkDrawBox(TdSpriteBatch* sb, Vector2 pos, float w, float h, const Color& c, float depth = 1, float rot = 0, const Vector2* origin = nullptr, int32 flip = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* m = nullptr) { tdVkDrawBox(sb, pos.x, pos.y, w, h, c, depth, rot, origin, flip, texture, src, m); }
inline void tdVkDrawBox(TdSpriteBatch* sb, TdRect rect, const Color& c, float depth = 1, float rot = 0, const Vector2* origin = nullptr, int32 flip = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* m = nullptr) { tdVkDrawBox(sb, rect.x, rect.y, rect.w, rect.h, c, depth, rot, origin, flip, texture, src, m); }
inline void tdVkDrawBoxO(TdSpriteBatch* sb, TdRect rect, const Color& c, float border_size, const Color& border_color, float depth = 1, float rot = 0, const Vector2* origin = nullptr) { tdVkDrawBoxO(sb, rect.x, rect.y, rect.w, rect.h, c, border_size, border_color, depth, rot, origin); }

void tdVkDrawCircle(TdSpriteBatch*, float x, float y, float r, int32 tess, const Color&, float depth = 1, float rot = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* = nullptr);
void tdVkDrawRing(TdSpriteBatch *, float x, float y, float r, float s, int32 tess, const Color &, float depth = 1, float rot = 0);

void tdVkDrawText(TdSpriteBatch * sb, const char *text, int text_len, float x, float y, const Color &color, float depth = 1, float scale = 1, float rot = 0, const Vector2 *origin = nullptr);
Vector2 tdVkDrawTextCentered(TdSpriteBatch *, const char *text, float x, float y, const Color &, float depth = 1, float scale = 1, float rot = 0, const Vector2 *origin = nullptr);
TdPoint2 tdVkDrawTextRight(TdSpriteBatch *, const char *text, const TdRect &rect, const Color &, float depth = 1, float scale = 1, float rot = 0, const Vector2 *origin = nullptr);

}
