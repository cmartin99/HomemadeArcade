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
		float x, y;
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
		uint32 texture_count;
		TextureData textures[MAX_TEXTURES];
		VkDescriptorSet descriptor_sets[MAX_TEXTURES];
		VkDescriptorSet descriptor_set_ubo;
		VkPipeline pso;
		VkDescriptorPool descriptor_pool_image;
		VkDescriptorPool descriptor_pool_ubo;
		VkPipelineLayout pipeline_layout;
	} font_bmp, font_df;
};

void tdVkSpriteBatchInit(TdSpriteBatch&, TdVkInstance&, uint32 = TdSpriteBatch::MAX_VERTICES);
void tdVkSpriteBatchPresent(TdSpriteBatch&, VkCommandBuffer);
void tdVkSpriteBatchGetTextSize(TdSpriteBatch *sb, const char *text, int32 text_len, Vector2 &result);

void tdVkDrawText(TdSpriteBatch&, const char* text, int text_len, float x, float y, const Color&, float scale);
void tdVkDrawText(TdSpriteBatch&, const char* text, int text_len, const Vector2& pos, const Color&, float scale);
void tdVkDrawTextDF(TdSpriteBatch&, const char* text, int text_len, float x, float y, const Color&, float scale);
void tdVkDrawTextDF(TdSpriteBatch&, const char* text, int text_len, const Vector2& pos, const Color&, float scale);
void tdVkDrawTextCenteredDF(TdSpriteBatch*, const char* text, const TdRect& rect, const Color&, float scale);

void tdVkDrawBox(TdSpriteBatch &, const Matrix &, float w, float h, const Color&);
void tdVkDrawBox(TdSpriteBatch &, const Matrix &, float wt, float wb, float h, const Color&);
void tdVkDrawBox(TdSpriteBatch&, float x, float y, float w, float h, const Color&);
void tdVkDrawBox(TdSpriteBatch&, float x, float y, float w, float h, const Color&, const TdVkTexture& texture, const TdRect* src);
void tdVkDrawBox(TdSpriteBatch&, float x, float y, float w, float h, const Color&, float border_size, const Color& border_color);
void tdVkDrawBoxDF(TdSpriteBatch&, float x, float y, float w, float h, const Color&);
void tdVkDrawBoxDF(TdSpriteBatch&, float x, float y, float w, float h, const Color&, float border_size, const Color& border_color);
ALWAYS_INLINE void tdVkDrawBox(TdSpriteBatch& sb, const TdRect& r, const Color& c) { tdVkDrawBox(sb, r.x, r.y, r.w, r.h, c); }
ALWAYS_INLINE void tdVkDrawBox(TdSpriteBatch& sb, const TdRect& r, const Color& c, const TdVkTexture& texture, const TdRect* src) { tdVkDrawBox(sb, r.x, r.y, r.w, r.h, c, texture, src); }
ALWAYS_INLINE void tdVkDrawBox(TdSpriteBatch& sb, const TdRect& r, const Color& c, float border_size, const Color& border_color) { tdVkDrawBox(sb, r.x, r.y, r.w, r.h, c, border_size, border_color); }
ALWAYS_INLINE void tdVkDrawBoxDF(TdSpriteBatch& sb, const TdRect& r, const Color& c) { tdVkDrawBoxDF(sb, r.x, r.y, r.w, r.h, c); }
ALWAYS_INLINE void tdVkDrawBoxDF(TdSpriteBatch& sb, const TdRect& r, const Color& c, float border_size, const Color& border_color) { tdVkDrawBoxDF(sb, r.x, r.y, r.w, r.h, c, border_size, border_color); }

void tdVkDrawRing(TdSpriteBatch &, float x, float y, float r, float s, uint32 tess, const Color &);

inline void tdVkDrawText(TdSpriteBatch* sb, const char* text, int text_len, float x, float y, const Color& color, float scale) { assert(sb); tdVkDrawText(*sb, text, text_len, x, y, color, scale); }
inline void tdVkDrawText(TdSpriteBatch* sb, const char* text, int text_len, const Vector2& pos, const Color& color, float scale) { assert(sb); tdVkDrawText(*sb, text, text_len, pos, color, scale); }
inline void tdVkDrawTextDF(TdSpriteBatch* sb, const char* text, int text_len, float x, float y, const Color& color, float scale) { assert(sb); tdVkDrawTextDF(*sb, text, text_len, x, y, color, scale); }
inline void tdVkDrawTextDF(TdSpriteBatch* sb, const char* text, int text_len, const Vector2& pos, const Color& color, float scale) { assert(sb); tdVkDrawTextDF(*sb, text, text_len, pos, color, scale); }
inline void tdVkDrawBox(TdSpriteBatch* sb, const Matrix& m, float w, float h, const Color& color) {	assert(sb);	tdVkDrawBox(*sb, m, w, h, color); }
inline void tdVkDrawBox(TdSpriteBatch* sb, const Matrix& m, float wt, float wb, float h, const Color& color) {	assert(sb);	tdVkDrawBox(*sb, m, wb, wt, h, color); }
inline void tdVkDrawBox(TdSpriteBatch * sb, float x, float y, float w, float h, const Color &color)	{ assert(sb); tdVkDrawBox(*sb, x, y, w, h, color); }
inline void tdVkDrawBox(TdSpriteBatch* sb, float x, float y, float w, float h, const Color& color, float border_size, const Color& border_color) { assert(sb); tdVkDrawBox(*sb, x, y, w, h, color, border_size, border_color); }
inline void tdVkDrawBox(TdSpriteBatch* sb, const TdRect& r, const Color& color) { assert(sb); tdVkDrawBox(*sb, r, color); }
inline void tdVkDrawBox(TdSpriteBatch* sb, const TdRect& r, const Color& color, float border_size, const Color& border_color) { assert(sb); tdVkDrawBox(*sb, r, color, border_size, border_color); }
inline void tdVkDrawBoxDF(TdSpriteBatch* sb, float x, float y, float w, float h, const Color& color) { assert(sb); tdVkDrawBoxDF(*sb, x, y, w, h, color); }
inline void tdVkDrawBoxDF(TdSpriteBatch* sb, float x, float y, float w, float h, const Color& color, float border_size, const Color& border_color) { assert(sb); tdVkDrawBoxDF(*sb, x, y, w, h, color, border_size, border_color); }
}
