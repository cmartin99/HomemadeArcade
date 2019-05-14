#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdSpriteBatch
{
	const static int MAX_TEXTURES = 16;
	static int MAX_VERTICES;
	TdVkInstance* vulkan;
	uint32 max_vertices;

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
	} ubo_info_fs;

	struct
	{
		Vector4 outlineColor = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
		float outlineWidth = 0.6f;
		float outline = true;
	} ubo_fs;

	struct TextureData
	{
		TdVkTexture* texture;
		VkBuffer vb_buffer;
		VkDeviceMemory vb_gpu_mem;
		VertexSprite* vb_cpu_mem;
		uint32 vertex_count;
		uint32 data_size;
	};

	struct FontType
	{
		int32 texture_count;
		TextureData textures[MAX_TEXTURES];
		VkDescriptorSet descriptor_sets[MAX_TEXTURES];
		VkDescriptorSet descriptor_set_ubo;
		VkPipeline pso;
		VkDescriptorPool descriptor_pool_image;
		VkDescriptorPool descriptor_pool_ubo;
		VkPipelineLayout pipeline_layout;
	} font_bmp, font_df;
};

void tdVkSpriteBatchInit(TdVkInstance*, TdSpriteBatch*, uint32 = TdSpriteBatch::MAX_VERTICES);
void tdVkSpriteBatchPresent(TdSpriteBatch*, VkCommandBuffer);
Vector2 tdVkSpriteBatchGetTextSize(TdSpriteBatch *sb, const char *text, int32 text_len, float scale = 1.0f);

void tdVkDrawBox(TdSpriteBatch*, float x, float y, float w, float h, const Color&, float depth = 1, float rot = 0, const Vector2* origin = nullptr, int32 flip = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* = nullptr);
inline void tdVkDrawBox(TdSpriteBatch* sb, TdRect rect, const Color& c, float depth = 1, float rot = 0, const Vector2* origin = nullptr, int32 flip = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* m = nullptr) { tdVkDrawBox(sb, rect.x, rect.y, rect.w, rect.h, c, depth, rot, origin, flip, texture, src, m); }
void tdVkDrawBoxO(TdSpriteBatch*, float x, float y, float w, float h, const Color&, float border_size, const Color& border_color, float depth = 1, float rot = 0, const Vector2* origin = nullptr);
inline void tdVkDrawBoxO(TdSpriteBatch* sb, TdRect rect, const Color& c, float border_size, const Color& border_color, float depth = 1, float rot = 0, const Vector2* origin = nullptr) { tdVkDrawBoxO(sb, rect.x, rect.y, rect.w, rect.h, c, border_size, border_color, depth, rot, origin); }
void tdVkDrawBoxDF(TdSpriteBatch *, float x, float y, float w, float h, const Color &, float rot = 0, const Vector2 *origin = nullptr, int32 flip = 0, const TdVkTexture *texture = nullptr, const TdRect *src = nullptr, const Matrix * = nullptr);
void tdVkDrawBoxODF(TdSpriteBatch *, float x, float y, float w, float h, const Color &, float border_size, const Color &border_color, float rot = 0, const Vector2 *origin = nullptr);
void tdVkDrawBoxX(TdSpriteBatch *, float wt, float wb, float h, const Color &, const Matrix *);

void tdVkDrawCircle(TdSpriteBatch*, float x, float y, float r, int32 tess, const Color&, float depth = 1, float rot = 0, const TdVkTexture* texture = nullptr, const TdRect* src = nullptr, const Matrix* = nullptr);
void tdVkDrawRing(TdSpriteBatch *, float x, float y, float r, float s, int32 tess, const Color &, float depth = 1, float rot = 0);

void tdVkDrawText(TdSpriteBatch * sb, const char *text, int text_len, float x, float y, const Color &color, float depth = 1, float scale = 1, float rot = 0, const Vector2 *origin = nullptr);
void tdVkDrawTextDF(TdSpriteBatch * sb, const char *text, int text_len, float x, float y, const Color &color, float depth = 1, float scale = 1, float rot = 0, const Vector2 *origin = nullptr);
Vector2 tdVkDrawTextCenteredDF(TdSpriteBatch *, const char *text, float x, float y, const Color &, float depth = 1, float scale = 1, float rot = 0, const Vector2 *origin = nullptr);
TdPoint2 tdVkDrawTextRightDF(TdSpriteBatch *, const char *text, const TdRect &rect, const Color &, float scale, float rot = 0, const Vector2 *origin = nullptr);

}
