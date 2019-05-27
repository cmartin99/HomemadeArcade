#include "TdSpriteBatch.h"
#include <string.h>
#include <sstream>
#include <fstream>
#include <iosfwd>

namespace eng {

struct BmChar
{
	uint32 x, y;
	uint32 width;
	uint32 height;
	int32 xoffset;
	int32 yoffset;
	int32 xadvance;
	uint32 page;
};

static BmChar font_chars_bmp[256];
static BmChar font_chars_df[256];

ALWAYS_INLINE BmChar* GetFontChars(const TdSpriteBatch* sprite_batch)
{
	return sprite_batch->distance_field 
		? &font_chars_df[0] 
		: &font_chars_bmp[0];
}

int32 NextValuePair(std::stringstream *stream)
{
	std::string pair;
	*stream >> pair;
	uint32 spos = pair.find("=");
	std::string value = pair.substr(spos + 1);
	int32 val = std::stoi(value);
	return val;
}

void ParseBmFont(const char* filename, BmChar* font_chars)
{
	std::filebuf file_buffer;
	file_buffer.open(filename, std::ios::in);
	std::istream istream(&file_buffer);
	assert(istream.good());

	while (!istream.eof())
	{
		std::string line;
		std::stringstream line_stream;
		std::getline(istream, line);
		line_stream << line;

		std::string info;
		line_stream >> info;

		if (info == "char")
		{
			std::string pair;
			uint32 charid = NextValuePair(&line_stream);
			if (charid < 256)
			{
				font_chars[charid].x = NextValuePair(&line_stream);
				font_chars[charid].y = NextValuePair(&line_stream);
				font_chars[charid].width = NextValuePair(&line_stream);
				font_chars[charid].height = NextValuePair(&line_stream);
				font_chars[charid].xoffset = NextValuePair(&line_stream);
				font_chars[charid].yoffset = NextValuePair(&line_stream);
				font_chars[charid].xadvance = NextValuePair(&line_stream);
				font_chars[charid].page = NextValuePair(&line_stream);
			}
		}
	}
}

void UpdateUniformBuffers(TdSpriteBatch* sprite_batch)
{
	assert(sprite_batch);
	VkResult err;
	TdVkInstance* vulkan = sprite_batch->vulkan;

	void* cpu_mem;
	err = vkMapMemory(vulkan->device, sprite_batch->ubo_info_fs.gpu_mem, 0, sizeof(sprite_batch->ubo_fs), 0, &cpu_mem);
	if (err)
	{
		tdDisplayError("vkMapMemory", err);
		return;
	}

	memcpy(cpu_mem, &sprite_batch->ubo_fs, sizeof(sprite_batch->ubo_fs));
	vkUnmapMemory(vulkan->device, sprite_batch->ubo_info_fs.gpu_mem);
}

int16 GetTextureIndex(TdSpriteBatch* sprite_batch, const TdVkTexture* texture)
{
	assert(sprite_batch);

	int16 t = -1;
	if (texture)
	{
		TdSpriteBatch::TextureData* tex_data = sprite_batch->textures.ptr;
		for (int32 i = 0; i < sprite_batch->textures.count; ++i, ++tex_data)
		{
			if (texture == tex_data->texture)
				return i;
		}

		if (sprite_batch->textures.count < sprite_batch->textures.cap)
		{
			tex_data = tdArrayPush(sprite_batch->textures);
			tex_data->texture = (TdVkTexture*)texture;
			tex_data->vertex_count = 0;
			t = sprite_batch->textures.count - 1;
		}
	}

	return t;
}

TdSpriteBatch::Batch* GetNextBatch(TdSpriteBatch* sprite_batch, int16 tex_id)
{
	if (sprite_batch->batches.count == 0 || sprite_batch->batches[sprite_batch->batches.count - 1].tex_id != tex_id)
	{
		TdSpriteBatch::Batch* batch = tdArrayPush(sprite_batch->batches);
		batch->tex_id = tex_id;
		batch->vertex_count = 0;
		batch->vertex_offset = sprite_batch->vertex_count;
		return batch;
	}

	return sprite_batch->batches.ptr + sprite_batch->batches.count - 1;
}

ALWAYS_INLINE TdSpriteBatch::Batch* GetNextBatch(TdSpriteBatch* sprite_batch, const TdVkTexture* texture)
{
	return GetNextBatch(sprite_batch, GetTextureIndex(sprite_batch, texture));
}

Vector2 tdVkSpriteBatchGetTextSize(TdSpriteBatch* sprite_batch, const char* text, int32 text_len, float scale)
{
	Vector2 result = {0, 0};
	if (!text) return result;

	uint32 font_size = 36;
	BmChar* font_chars = font_chars_bmp;
	BmChar* charInfo;
	Vector2 store(0, font_size);
	if (text_len == 0) text_len = INT32_MAX;

	for (int32 i = 0; i < text_len && text[i] > 0; ++i)
	{
		if (text[i] != 10)
		{
			charInfo = &font_chars[text[i]];
			if (charInfo->width > 0)
				result.x += /* charInfo->width + charInfo->xoffset ? */ charInfo->xadvance;
			else
				result.x += font_size;
		}
		else
		{
			store.y += font_size;
			if (result.x > store.x) store.x = result.x;
			result.x = result.y = 0;
		}
	}

	if (store.x > result.x) result.x = store.x;
	result.y += store.y;
	result.x *= scale;
	result.y *= scale;
	return result;
}

void tdVkDrawTextCore(TdSpriteBatch* sprite_batch, const char* text, int32 text_len, float x, float y, const Color& color, float depth, float scale, float rot, const Vector2* origin, BmChar* font_chars)
{
	assert(sprite_batch);
	if (!text) return;
	if (text_len == 0) text_len = INT32_MAX;

	uint32 vp_width = sprite_batch->vulkan->surface_width;
	uint32 vp_height = sprite_batch->vulkan->surface_height;

	if (x >= vp_width || y > vp_height || y < -100) return;
	if (up3.y < 0) y = vp_height - y;

	TdSpriteBatch::Batch* batch = GetNextBatch(sprite_batch, (int16)0);
	TdSpriteBatch::TextureData* tex_data = sprite_batch->textures.ptr + batch->tex_id;
	TdSpriteBatch::VertexSprite* pv = sprite_batch->vb_cpu_mem + batch->vertex_offset + batch->vertex_count;
	TdSpriteBatch::VertexSprite* first_pv = pv;

	float w = tex_data->texture->width;
	float h = tex_data->texture->height;
	float font_size = 36 * scale;
	float base_line = 29 * scale;
	//y -= base_line;
	depth = 1 - depth;

	float ox = x;
	const char *txt = text;
	const char *txt_end = txt + text_len;
	char t;
	while (txt < txt_end)
	{
		t = *txt;
		if (!t)
			break;

		if (t != 10)
		{
			BmChar* charInfo = font_chars + t;
			float advance = font_size;

			if (charInfo->width > 0)
			{
				float charw = charInfo->width;
				float charh = charInfo->height;
				float dimx = charw * scale;
				float dimy = charh * scale;

				float u1 = charInfo->x / w;
				float u2 = (charInfo->x + charw) / w;
				float v2 = charInfo->y / h;
				float v1 = (charInfo->y + charh) / h;

				float xo = charInfo->xoffset * scale;
				float yo = charInfo->yoffset * scale;

				float xx = (x + xo) / vp_width * 2.0f - 1.0f;
				float yy = ((y - yo) / vp_height * 2.0f - 1.0f) * up3.y;
				float y2 = yy;

				pv->x = xx;
				pv->y = yy;
				pv->z = depth;
				pv->type = 2;
				pv->color.r = color.r;
				pv->color.g = color.g;
				pv->color.b = color.b;
				pv->color.a = color.a;
				pv->u = u1;
				pv->v = v2;

				yy = ((y - yo - dimy) / vp_height * 2.0f - 1.0f) * up3.y;

				++pv;
				pv->x = xx;
				pv->y = yy;
				pv->z = depth;
				pv->type = 2;
				pv->color.r = color.r;
				pv->color.g = color.g;
				pv->color.b = color.b;
				pv->color.a = color.a;
				pv->u = u1;
				pv->v = v1;

				xx = (x + dimx) / vp_width * 2.0f - 1.0f;

				++pv;
				pv->x = xx;
				pv->y = yy;
				pv->z = depth;
				pv->type = 2;
				pv->color.r = color.r;
				pv->color.g = color.g;
				pv->color.b = color.b;
				pv->color.a = color.a;
				pv->u = u2;
				pv->v = v1;

				++pv;
				pv->x = xx;
				pv->y = y2;
				pv->z = depth;
				pv->type = 2;
				pv->color.r = color.r;
				pv->color.g = color.g;
				pv->color.b = color.b;
				pv->color.a = color.a;
				pv->u = u2;
				pv->v = v2;

				++pv;
				advance = charInfo->xadvance * scale;
			}

			x += advance;
		}
		else
		{
			y -= font_size;
			x = ox;
		}
		++txt;
	}

	batch->vertex_count += (pv - first_pv);
	tex_data->vertex_count += (pv - first_pv);
	sprite_batch->vertex_count += (pv - first_pv);

#ifdef _PROFILE_
	++sprite_batch->debug_info.td_draw_calls;
#endif
}

void tdVkDrawText(TdSpriteBatch *sprite_batch, const char *text, int text_len, float x, float y, const Color &color, float depth, float scale, float rot, const Vector2* origin)
{
	tdVkDrawTextCore(sprite_batch, text, text_len, x, y, color, depth, scale, rot, origin, GetFontChars(sprite_batch));
}

Vector2 tdVkDrawTextCentered(TdSpriteBatch* sprite_batch, const char* text, float x, float y, const Color& color, float depth, float scale, float rot, const Vector2* origin)
{
	assert(sprite_batch);
	Vector2 text_size = tdVkSpriteBatchGetTextSize(sprite_batch, text, 0, scale);
	Vector2 pos = {x - text_size.x * 0.5f, y - text_size.y * 0.5f + 1};
	tdVkDrawTextCore(sprite_batch, text, 0, pos.x, pos.y, color, depth, scale, rot, origin, GetFontChars(sprite_batch));
	return pos;
}

TdPoint2 tdVkDrawTextRight(TdSpriteBatch* sprite_batch, const char* text, const TdRect& rect, const Color& color, float depth, float scale, float rot, const Vector2* origin)
{
	assert(sprite_batch);
	Vector2 text_size = tdVkSpriteBatchGetTextSize(sprite_batch, text, 0, scale);
	TdPoint2 pos = tdRectCenter(rect, text_size.x, text_size.y);
	--pos.y;
	pos.x = rect.x + rect.w - text_size.x;
	tdVkDrawTextCore(sprite_batch, text, 0, pos.x, pos.y, color, depth, scale, rot, origin, GetFontChars(sprite_batch));
	return pos;
}

void tdVkDrawBoxX(TdSpriteBatch* sprite_batch, float wt, float wb, float h, const Color& color, const Matrix* mat, float depth)
{
	assert(sprite_batch);
	assert(mat);
	if (color.a == 0) return;

	assert(sprite_batch->vertex_count + 4 <= sprite_batch->max_vertices);
	TdSpriteBatch::Batch* batch = GetNextBatch(sprite_batch, -1);
	TdSpriteBatch::VertexSprite* pv = sprite_batch->vb_cpu_mem + batch->vertex_offset + batch->vertex_count;
	TdSpriteBatch::VertexSprite* first_pv = pv;

	Vector4 local(wb * -0.5f, h * 0.5f, 0, 1);
	Vector4 result = *mat * local;
	depth = 1 - depth;

	pv->x = result.x;
	pv->y = result.y;
	pv->z = depth;
	pv->type = 0;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;

	local.x = wt * -0.5f;
	local.y -= h;
	result = *mat * local;

	++pv;
	pv->x = result.x;
	pv->y = result.y;
	pv->z = depth;
	pv->type = 0;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;

	local.x += wt;
	result = *mat * local;

	++pv;
	pv->x = result.x;
	pv->y = result.y;
	pv->z = depth;
	pv->type = 0;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;

	local.x = wb * 0.5f;
	local.y += h;
	result = *mat * local;

	++pv;
	pv->x = result.x;
	pv->y = result.y;
	pv->z = depth;
	pv->type = 0;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;
	++pv;

	batch->vertex_count += (pv - first_pv);
	sprite_batch->vertex_count += (pv - first_pv);

#ifdef _PROFILE_
	++sprite_batch->debug_info.td_draw_calls;
#endif
}

void tdVkDrawBoxCore(TdSpriteBatch* sprite_batch, float x, float y, float w, float h, const Color& color, float depth, float rot, const Vector2* origin, int32 flip, const TdVkTexture* texture, const TdRect* src)
{
	assert(sprite_batch);
	if (color.a == 0) return;

	uint32 vp_width = sprite_batch->vulkan->surface_width;
	uint32 vp_height = sprite_batch->vulkan->surface_height;
	if (x >= vp_width || x + w <= 0 || y > vp_height || y + h <= 0) return;

	assert(sprite_batch->vertex_count + 4 <= sprite_batch->max_vertices);
	TdSpriteBatch::Batch* batch = GetNextBatch(sprite_batch, texture);
	TdSpriteBatch::TextureData* tex_data = nullptr;
	TdSpriteBatch::VertexSprite* pv = sprite_batch->vb_cpu_mem + batch->vertex_offset + batch->vertex_count;
	TdSpriteBatch::VertexSprite* first_pv = pv;

	float u0 = 0, u1 = 1;
	float v0 = 0, v1 = 1;
	float type = 0;

	if (batch->tex_id >= 0)
	{
		type = 1;
		tex_data = sprite_batch->textures.ptr + batch->tex_id;
		if (src)
		{
			float tw = (float)texture->width;
			float th = (float)texture->height;
			u0 = (float)src->x / tw;
			v0 = (float)src->y / th;
			u1 = (float)(src->x + src->w) / tw;
			v1 = (float)(src->y + src->h) / th;
		}
		if (flip & 0x01) { float t = u1; u1 = u0; u0 = t; }
		if (flip & 0x02) { float t = v1; v1 = v0; v0 = t; }
	}

	float x1, y1, x2, y2, x3, y3, x4, y4;

	if (!rot)
	{
		x1 = x;
		x2 = x;
		x3 = x + w;
		x4 = x3;
		y1 = y + h;
		y2 = y;
		y3 = y;
		y4 = y1;
	}
	else
	{
		//x0+(x−x0)cosϕ+(y−y0)sinϕ
		//y0−(x−x0)sinϕ+(y−y0)cosϕ
		float cx = x + (origin ? origin->x : w * 0.5f);
		float cy = y + (origin ? origin->y : h * 0.5f);
		float cos_rot = cos(rot);
		float sin_rot = sin(rot);
		float x_cx = (x - cx);
		float xw_cx = (x + w) - cx;
		float y_cy = (y - cy);
		float yh_cy = (y + h) - cy;
		x1 = cx + x_cx * cos_rot + yh_cy * sin_rot;
		x2 = cx + x_cx * cos_rot + y_cy * sin_rot;
		x3 = cx + xw_cx * cos_rot + y_cy * sin_rot;
		x4 = cx + xw_cx * cos_rot + yh_cy * sin_rot;
		y1 = cy - x_cx * sin_rot + yh_cy * cos_rot;
		y2 = cy - x_cx * sin_rot + y_cy * cos_rot;
		y3 = cy - xw_cx * sin_rot + y_cy * cos_rot;
		y4 = cy - xw_cx * sin_rot + yh_cy * cos_rot;
	}

	float vpw = 2.0f / vp_width;
	float vph = 2.0f / vp_height;
	depth = 1 - depth;

	pv->x = x1 * vpw - 1.0f;
	pv->y = y1 * vph - 1.0f;
	pv->z = depth;
	pv->type = type;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;
	pv->u = u0;
	pv->v = v1;

	++pv;
	pv->x = x2 * vpw - 1.0f;
	pv->y = y2 * vph - 1.0f;
	pv->z = depth;
	pv->type = type;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;
	pv->u = u0;
	pv->v = v0;

	++pv;
	pv->x = x3 * vpw - 1.0f;
	pv->y = y3 * vph - 1.0f;
	pv->z = depth;
	pv->type = type;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;
	pv->u = u1;
	pv->v = v0;

	++pv;	
	pv->x = x4 * vpw - 1.0f;
	pv->y = y4 * vph - 1.0f;
	pv->z = depth;
	pv->type = type;
	pv->color.r = color.r;
	pv->color.g = color.g;
	pv->color.b = color.b;
	pv->color.a = color.a;
	pv->u = u1;
	pv->v = v1;
	++pv;

	batch->vertex_count += (pv - first_pv);
	if (tex_data) tex_data->vertex_count += (pv - first_pv);
	sprite_batch->vertex_count += (pv - first_pv);

#ifdef _PROFILE_
	++sprite_batch->debug_info.td_draw_calls;
#endif
}

void tdVkDrawBoxCore(TdSpriteBatch* sprite_batch, float x, float y, float w, float h, const Color& color, float border_size, const Color& border_color, float depth, float rot, const Vector2* origin)
{
	assert(sprite_batch);
	int border_size2 = border_size + border_size;

	if (border_size > 0)
	{
		tdVkDrawBoxCore(sprite_batch, x, y, w, border_size, border_color, depth, rot, origin, 0, NULL, NULL);

		if (h >= border_size2)
		{
			tdVkDrawBoxCore(sprite_batch, x, y + border_size, border_size, h - border_size2, border_color, depth, rot, origin, 0, NULL, NULL);
			if (w >= border_size2)
			{
				tdVkDrawBoxCore(sprite_batch, x + w - border_size, y + border_size, border_size, h - border_size2, border_color, depth, rot, origin, 0, NULL, NULL);
			}
		}

		if (h > border_size)
		{
			tdVkDrawBoxCore(sprite_batch, x, y + h - border_size, w, border_size, border_color, depth, rot, origin, 0, NULL, NULL);
		}

		x += border_size;
		y += border_size;
		w -= border_size2;
		h -= border_size2;
	}

	if (w > 0 && h > 0)
	{
		tdVkDrawBoxCore(sprite_batch, x, y, w, h, color, depth, rot, origin, 0, NULL, NULL);
	}
}

void tdVkDrawBox(TdSpriteBatch* sprite_batch, float x, float y, float w, float h, const Color& color, float depth, float rot, const Vector2* origin, int32 flip, const TdVkTexture* texture, const TdRect* src_rect, const Matrix* mat)
{
	tdVkDrawBoxCore(sprite_batch, x, y, w, h, color, depth, rot, origin, flip, texture, src_rect);
}

void tdVkDrawBoxO(TdSpriteBatch* sprite_batch, float x, float y, float w, float h, const Color& color, float border_size, const Color& border_color, float depth, float rot, const Vector2* origin)
{
	tdVkDrawBoxCore(sprite_batch, x, y, w, h, color, border_size, border_color, depth, rot, origin);
}

void tdVkDrawCircle(TdSpriteBatch* sprite_batch, float x, float y, float r, int32 tess, const Color& color, float depth, float rot, const TdVkTexture* texture, const TdRect* src, const Matrix* mat)
{
	assert(sprite_batch);
	assert(r > 0);
	assert(tess > 3);
	if (color.a == 0) return;

	uint32 vp_width = sprite_batch->vulkan->surface_width;
	uint32 vp_height = sprite_batch->vulkan->surface_height;
	if (x - r >= vp_width || x + r <= 0 || y - r > vp_height || y + r <= 0) return;

	TdSpriteBatch::Batch* batch = GetNextBatch(sprite_batch, texture);
	TdSpriteBatch::TextureData* tex_data = texture ? sprite_batch->textures.ptr + batch->tex_id : nullptr;
	TdSpriteBatch::VertexSprite* pv = sprite_batch->vb_cpu_mem + batch->vertex_offset + batch->vertex_count;
	TdSpriteBatch::VertexSprite* first_pv = pv;

	float vpw = 2.0f / vp_width;
	float vph = 2.0f / vp_height;
	float x1, y1;
	const float pi2 = two_pi<float>();
	const float pi2t = pi2 / tess;
	int32 te = (tess % 2 == 1) ? tess + 1 : tess;
	int32 tess2 = te / 2;
	int32 i = 0;
	depth = 1 - depth;
	float type = batch->tex_id >= 0 ? 1 : 0;

	for (int32 t = 0; t < tess2; ++t)
	{
		pv->x = x * vpw - 1.0f;
		pv->y = y * vph - 1.0f;
		pv->z = depth;
		pv->type = type;
		pv->color.r = color.r;
		pv->color.g = color.g;
		pv->color.b = color.b;
		pv->color.a = color.a;
		pv->u = 0.5f;
		pv->v = 0.5F;
		++pv;

		float angle = i * pi2t + rot;
		x1 = x + (float)cos(angle) * r;
		y1 = y + (float)sin(angle) * r;
		pv->x = x1 * vpw - 1.0f;
		pv->y = y1 * vph - 1.0f;
		pv->z = depth;
		pv->type = type;
		pv->color.r = color.r;
		pv->color.g = color.g;
		pv->color.b = color.b;
		pv->color.a = color.a;
		if (type) {
			pv->u = 0.5f + (float)cos(angle) * 0.5f;
			pv->v = 0.5f + (float)sin(angle) * 0.5f;
		}
		++pv;

		angle = (i + 1) * pi2t + rot;
		x1 = x + (float)cos(angle) * r;
		y1 = y + (float)sin(angle) * r;
		pv->x = x1 * vpw - 1.0f;
		pv->y = y1 * vph - 1.0f;
		pv->z = depth;
		pv->type = type;
		pv->color.r = color.r;
		pv->color.g = color.g;
		pv->color.b = color.b;
		pv->color.a = color.a;
		if (type) {
			pv->u = 0.5f + (float)cos(angle) * 0.5f;
			pv->v = 0.5f + (float)sin(angle) * 0.5f;
		}
		++pv;

		angle = ((i + 2) % tess) * pi2t + rot;
		x1 = x + (float)cos(angle) * r;
		y1 = y + (float)sin(angle) * r;
		pv->x = x1 * vpw - 1.0f;
		pv->y = y1 * vph - 1.0f;
		pv->z = depth;
		pv->type = type;
		pv->color.r = color.r;
		pv->color.g = color.g;
		pv->color.b = color.b;
		pv->color.a = color.a;
		if (type) {
			pv->u = 0.5f + (float)cos(angle) * 0.5f;
			pv->v = 0.5f + (float)sin(angle) * 0.5f;
		}
		++pv;
		i += 2;
	}

	batch->vertex_count += (pv - first_pv);
	if (tex_data) tex_data->vertex_count += (pv - first_pv);
	sprite_batch->vertex_count += (pv - first_pv);

#ifdef _PROFILE_
	++sprite_batch->debug_info.td_draw_calls;
#endif
}

void tdVkDrawRing(TdSpriteBatch* sprite_batch, float x, float y, float r, float s, int32 tess, const Color& color, float depth, float rot)
{
	assert(sprite_batch);
	const float pi2 = two_pi<float>();
	const float pi2t = pi2 / tess;
	float s2 = s * 0.5f;
	float x1, y1;	

	for (int32 i = 0; i < tess; ++i)
	{
		float angle = i * pi2t + rot;
		x1 = x - s2 + (float)cos(angle) * r;
		y1 = y - s2 + (float)sin(angle) * r;
		tdVkDrawBox(sprite_batch, x1, y1, s, s, color, depth);
	}
}

void tdVkSpriteBatchPresent(TdSpriteBatch* sprite_batch, VkCommandBuffer command_buffer)
{
	assert(sprite_batch);
	if (sprite_batch->vertex_count < 3) return;
	VkDeviceSize offsets[1] = { 0 };

	VkDescriptorImageInfo image_descriptor_info = {};
	image_descriptor_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	VkWriteDescriptorSet write_descriptor_set = {};
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor_set.dstBinding = 0;
	write_descriptor_set.pImageInfo = &image_descriptor_info;
	write_descriptor_set.descriptorCount = 1;

	TdSpriteBatch::TextureData* tex_data = sprite_batch->textures.ptr;
	for (int32 i = 0; i < sprite_batch->textures.count; ++i)
	{
		if (tex_data->vertex_count > 0)
		{
			image_descriptor_info.sampler = tex_data->texture->sampler;
			image_descriptor_info.imageView = tex_data->texture->image_view;
			write_descriptor_set.dstSet = sprite_batch->descriptor_sets[i];
			vkUpdateDescriptorSets(sprite_batch->vulkan->device, 1, &write_descriptor_set, 0, NULL);
			tex_data->vertex_count = 0;
		}
		++tex_data;
	}

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_batch->pso);

	TdSpriteBatch::Batch* batch = sprite_batch->batches.ptr;
	for (int32 i = 0; i < sprite_batch->batches.count; ++i)
	{
		if (batch->vertex_count > 0)
		{
			if (batch->tex_id >= 0)
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_batch->pipeline_layout, 0, 1, &sprite_batch->descriptor_sets[batch->tex_id], 0, NULL);

			uint32 indice_count = batch->vertex_count / 4 * 6;
			tdVkBindIndexBufferTri(sprite_batch->vulkan, command_buffer, indice_count);
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &sprite_batch->vb_buffer, offsets);
			vkCmdDrawIndexed(command_buffer, indice_count, 1, 0, batch->vertex_offset, 0);
#ifdef _PROFILE_
			++sprite_batch->debug_info.batch_count;
			++sprite_batch->debug_info.vk_draw_calls;
#endif
		}
		++batch;
	}

	tdArrayClear(sprite_batch->batches);
	sprite_batch->vertex_count = 0;
}

bool AllocateVBMem(TdSpriteBatch* sprite_batch)
{
	assert(sprite_batch);

	VkResult err;
	size_t data_size = sizeof(TdSpriteBatch::VertexSprite) * sprite_batch->max_vertices;

	VkBufferCreateInfo buffer_create_info = {};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_create_info.size = data_size;

	TdVkInstance* vulkan = sprite_batch->vulkan;
	err = vkCreateBuffer(vulkan->device, &buffer_create_info, NULL, &sprite_batch->vb_buffer);
	if (err)
	{
		tdDisplayError("vkCreateBuffer", err);
		return false;
	}

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(vulkan->device, sprite_batch->vb_buffer, &mem_reqs);

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;

	if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc.memoryTypeIndex))
	{
		err = VK_ERROR_INITIALIZATION_FAILED;
		tdDisplayError("tdVkGetMemoryType", err);
		return false;
	}

	err = vkAllocateMemory(vulkan->device, &mem_alloc, NULL, &sprite_batch->vb_gpu_mem);
	if (err)
	{
		tdDisplayError("vkAllocateMemory", err);
		return false;
	}

	err = vkBindBufferMemory(vulkan->device, sprite_batch->vb_buffer, sprite_batch->vb_gpu_mem, 0);
	if (err)
	{
		vkFreeMemory(vulkan->device, sprite_batch->vb_gpu_mem, NULL);
		tdDisplayError("vkBindBufferMemory", err);
		return false;
	}

	err = vkMapMemory(vulkan->device, sprite_batch->vb_gpu_mem, 0, data_size, 0, (void**)&sprite_batch->vb_cpu_mem);
	if (err)
	{
		vkFreeMemory(vulkan->device, sprite_batch->vb_gpu_mem, NULL);
		tdDisplayError("vkMapMemory", err);
		return false;
	}

	sprite_batch->data_size = data_size;
	return true;
}

void tdVkSpriteBatchInit(TdVkInstance* vulkan, TdSpriteBatch* sprite_batch, TdMemoryArena* arena, bool distance_field, int32 max_vertices, int32 max_textures)
{
	assert(vulkan);
	assert(sprite_batch);
	assert(arena);
	assert(max_vertices >= 4);
	max_textures = max(2, max_textures);
	VkResult err;

	sprite_batch->vulkan = vulkan;
	sprite_batch->distance_field = distance_field;
	sprite_batch->max_vertices = max_vertices;
	if (!AllocateVBMem(sprite_batch)) return;

	sprite_batch->vertex_count = 0;
	tdArrayInit(sprite_batch->textures, *arena, max_textures);
	tdArrayInit(sprite_batch->batches, *arena, max_vertices / 4);
	sprite_batch->descriptor_sets = tdMalloc<VkDescriptorSet>(arena, max_textures);

	ParseBmFont("content/consolas_df.fnt", font_chars_df);
	ParseBmFont("content/consolas_bmp.fnt", font_chars_bmp);

	TdSpriteBatch::TextureData* tex_data = tdArrayPush(sprite_batch->textures); // first texture = font texture
	tex_data->texture = distance_field 
		? tdVkLoadTexture(vulkan, "content/textures/consolas_df.ktx", VK_FORMAT_R8G8B8A8_UNORM, true)
		: tdVkLoadTexture(vulkan, "content/textures/consolas_bmp.ktx", VK_FORMAT_R8G8B8A8_UNORM, true);

	VkMemoryRequirements mem_reqs = {};
	VkMemoryAllocateInfo mem_alloc_info = {};
	mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	if (distance_field)
	{
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_info.size = sizeof(sprite_batch->ubo_fs);

		err = vkCreateBuffer(vulkan->device, &buffer_info, NULL, &sprite_batch->ubo_info_fs.buffer);
		if (err)
		{
			tdDisplayError("vkCreateBuffer", err);
			return;
		}

		vkGetBufferMemoryRequirements(vulkan->device, sprite_batch->ubo_info_fs.buffer, &mem_reqs);
		mem_alloc_info.allocationSize = mem_reqs.size;
		if (!tdVkGetMemoryType(vulkan, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mem_alloc_info.memoryTypeIndex))
		{
			err = VK_ERROR_INITIALIZATION_FAILED;
			tdDisplayError("vkCreateBuffer", err);
			return;
		}

		err = vkAllocateMemory(vulkan->device, &mem_alloc_info, NULL, &sprite_batch->ubo_info_fs.gpu_mem);
		if (err)
		{
			tdDisplayError("vkAllocateMemory", err);
			return;
		}

		err = vkBindBufferMemory(vulkan->device, sprite_batch->ubo_info_fs.buffer, sprite_batch->ubo_info_fs.gpu_mem, 0);
		if (err)
		{
			tdDisplayError("vkAllocateMemory", err);
			return;
		}

		sprite_batch->ubo_info_fs.descriptor.offset = 0;
		sprite_batch->ubo_info_fs.descriptor.buffer = sprite_batch->ubo_info_fs.buffer;
		sprite_batch->ubo_info_fs.descriptor.range = buffer_info.size;
	}

	VkVertexInputBindingDescription vertex_input_binding_desc[1];
	vertex_input_binding_desc[0] = {};
	vertex_input_binding_desc[0].binding = 0;
	vertex_input_binding_desc[0].stride = sizeof(TdSpriteBatch::VertexSprite);
	vertex_input_binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertex_input_attr_desc[4];
	vertex_input_attr_desc[0] = {};
	vertex_input_attr_desc[0].location = 0;
	vertex_input_attr_desc[0].binding = 0;
	vertex_input_attr_desc[0].format = VK_FORMAT_R32_SFLOAT;
	vertex_input_attr_desc[0].offset = 0;
	vertex_input_attr_desc[1] = {};
	vertex_input_attr_desc[1].location = 1;
	vertex_input_attr_desc[1].binding = 0;
	vertex_input_attr_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attr_desc[1].offset = sizeof(float);
	vertex_input_attr_desc[2] = {};
	vertex_input_attr_desc[2].location = 2;
	vertex_input_attr_desc[2].binding = 0;
	vertex_input_attr_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertex_input_attr_desc[2].offset = sizeof(float) * 4;
	vertex_input_attr_desc[3] = {};
	vertex_input_attr_desc[3].location = 3;
	vertex_input_attr_desc[3].binding = 0;
	vertex_input_attr_desc[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertex_input_attr_desc[3].offset = sizeof(float) * 6;

	VkPipelineVertexInputStateCreateInfo vertex_input_state_info;
	vertex_input_state_info = {};
	vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_info.vertexBindingDescriptionCount = 1;
	vertex_input_state_info.pVertexBindingDescriptions = vertex_input_binding_desc;
	vertex_input_state_info.vertexAttributeDescriptionCount = 4;
	vertex_input_state_info.pVertexAttributeDescriptions = vertex_input_attr_desc;

	VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[1];
	descriptor_set_layout_bindings[0] = {};
	descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptor_set_layout_bindings[0].binding = 0;
	descriptor_set_layout_bindings[0].descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info = {};
	descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_info.pBindings = descriptor_set_layout_bindings;
	descriptor_set_layout_info.bindingCount = 1;

	VkDescriptorSetLayout descriptor_set_layout = {};
	VkDescriptorSetLayout descriptor_set_layout_df[2];

	if (distance_field)
	{
		err = vkCreateDescriptorSetLayout(vulkan->device, &descriptor_set_layout_info, NULL, descriptor_set_layout_df);
		if (err)
		{
			tdDisplayError("vkCreateDescriptorSetLayout", err);
			return;
		}

		descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptor_set_layout_bindings[0].binding = 1;

		err = vkCreateDescriptorSetLayout(vulkan->device, &descriptor_set_layout_info, NULL, descriptor_set_layout_df + 1);
		if (err)
		{
			tdDisplayError("vkCreateDescriptorSetLayout", err);
			return;
		}
	}
	else
	{
		err = vkCreateDescriptorSetLayout(vulkan->device, &descriptor_set_layout_info, NULL, &descriptor_set_layout);
		if (err)
		{
			tdDisplayError("vkCreateDescriptorSetLayout", err);
			return;
		}
	}

	VkDescriptorPoolSize pool_sizes[1];
	pool_sizes[0] = {};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptor_pool_info = {};
	descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_info.poolSizeCount = 1;
	descriptor_pool_info.pPoolSizes = pool_sizes;
	descriptor_pool_info.maxSets = max_textures;

	err = vkCreateDescriptorPool(vulkan->device, &descriptor_pool_info, NULL, &sprite_batch->descriptor_pool_image);
	if (err)
	{
		tdDisplayError("vkCreateDescriptorPool", err);
		return;
	}

	if (distance_field)
	{
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = 1;

		err = vkCreateDescriptorPool(vulkan->device, &descriptor_pool_info, NULL, &sprite_batch->descriptor_pool_ubo);
		if (err)
		{
			tdDisplayError("vkCreateDescriptorPool", err);
			return;
		}
	}

	VkDescriptorSetLayout* temp_layouts = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * max_textures);
	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
	descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_alloc_info.descriptorSetCount = max_textures;
	descriptor_set_alloc_info.pSetLayouts = temp_layouts;
	descriptor_set_alloc_info.descriptorPool = sprite_batch->descriptor_pool_image;

	for (uint64 i = 0; i < max_textures; ++i) temp_layouts[i] = distance_field ? descriptor_set_layout_df[0] : descriptor_set_layout;
	err = vkAllocateDescriptorSets(vulkan->device, &descriptor_set_alloc_info, sprite_batch->descriptor_sets);
	if (err)
	{
		tdDisplayError("vkAllocateDescriptorSets", err);
		return;
	}
	free(temp_layouts);

	if (distance_field)
	{
		descriptor_set_alloc_info.descriptorSetCount = 1;
		descriptor_set_alloc_info.pSetLayouts = descriptor_set_layout_df + 1;
		descriptor_set_alloc_info.descriptorPool = sprite_batch->descriptor_pool_ubo;
		err = vkAllocateDescriptorSets(vulkan->device, &descriptor_set_alloc_info, &sprite_batch->descriptor_set_ubo);
		if (err)
		{
			tdDisplayError("vkAllocateDescriptorSets", err);
			return;
		}
	}

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (distance_field)
	{
		pipeline_layout_info.setLayoutCount = 2;
		pipeline_layout_info.pSetLayouts = descriptor_set_layout_df;
		err = vkCreatePipelineLayout(vulkan->device, &pipeline_layout_info, NULL, &sprite_batch->pipeline_layout);
		if (err)
		{
			tdDisplayError("vkCreatePipelineLayout", err);
			return;
		}
	}
	else
	{
		pipeline_layout_info.setLayoutCount = 1;
		pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
		err = vkCreatePipelineLayout(vulkan->device, &pipeline_layout_info, NULL, &sprite_batch->pipeline_layout);
		if (err)
		{
			tdDisplayError("vkCreatePipelineLayout", err);
			return;
		}
	}

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info = {};
	input_assembly_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_state_info.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
	rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state_info.cullMode = VK_CULL_MODE_NONE;
	rasterization_state_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state_info.depthClampEnable = VK_TRUE;

	VkPipelineColorBlendAttachmentState blend_attachment_state = {};
	blend_attachment_state.colorWriteMask = 0xf;
	blend_attachment_state.blendEnable = VK_TRUE;
	blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
	blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo blend_state_info = {};
	blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_state_info.attachmentCount = 1;
	blend_state_info.pAttachments = &blend_attachment_state;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info = {};
	depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_info.depthTestEnable = VK_TRUE;
	depth_stencil_state_info.depthWriteEnable = VK_TRUE;
	depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth_stencil_state_info.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depth_stencil_state_info.front = depth_stencil_state_info.back;

	VkPipelineViewportStateCreateInfo viewport_state_info = {};
	viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_info.viewportCount = 1;
	viewport_state_info.pViewports = nullptr;
	viewport_state_info.scissorCount = 1;
	viewport_state_info.pScissors = nullptr;

	VkPipelineMultisampleStateCreateInfo multisample_state_info = {};
	multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkDynamicState dynamic_state_enables[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
	dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_info.pDynamicStates = dynamic_state_enables;
	dynamic_state_info.dynamicStateCount = 2;

	VkPipelineShaderStageCreateInfo shader_stages[2];

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.renderPass = vulkan->render_pass;
	pipeline_info.pVertexInputState = &vertex_input_state_info;
	pipeline_info.pInputAssemblyState = &input_assembly_state_info;
	pipeline_info.pRasterizationState = &rasterization_state_info;
	pipeline_info.pColorBlendState = &blend_state_info;
	pipeline_info.pMultisampleState = &multisample_state_info;
	pipeline_info.pViewportState = &viewport_state_info;
	pipeline_info.pDepthStencilState = &depth_stencil_state_info;
	pipeline_info.pDynamicState = &dynamic_state_info;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;

	shader_stages[0] = tdVkLoadShader(vulkan, "TdSpriteBatch.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shader_stages[1] = distance_field
		? tdVkLoadShader(vulkan, "TdSpriteBatchFontDF.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
		: tdVkLoadShader(vulkan, "TdSpriteBatchFontBmp.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	pipeline_info.layout = sprite_batch->pipeline_layout;
	err = vkCreateGraphicsPipelines(vulkan->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &sprite_batch->pso);
	if (err)
	{
		tdDisplayError("vkCreateGraphicsPipelines", err);
		return;
	}

	if (distance_field)
	{
		VkWriteDescriptorSet write_descriptor_set = {};
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_descriptor_set.dstBinding = 1;
		write_descriptor_set.pBufferInfo = &sprite_batch->ubo_info_fs.descriptor;
		write_descriptor_set.descriptorCount = 1;
		write_descriptor_set.dstSet = sprite_batch->descriptor_set_ubo;
		vkUpdateDescriptorSets(vulkan->device, 1, &write_descriptor_set, 0, NULL);
		UpdateUniformBuffers(sprite_batch);
	}
}

}