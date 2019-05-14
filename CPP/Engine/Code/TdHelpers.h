#pragma once
#include <stdlib.h>
#include "TdDataTypes.h"

namespace eng {

const double M_PI_2 = 1.57079632679489661923132169163975144;

std::string tdErrorString(VkResult);
char* tdLastErrorAsString(char *result_buffer, size_t result_buffer_len, bool concatenate = false);
void tdDisplayMessage(const char* msg);
void tdDisplayError(const char* msg);
void tdDisplayError(const char* msg, HRESULT);
void tdDisplayError(const char* msg, VkResult);
char *tdReadAllTextFileAndNullTerminate(const char *filename);
char* tdReadBinaryFile(const char* filename, size_t*);
char *tdReadBinaryFile(const char *filename, TdMemoryArena &, size_t *);
bool tdWriteBinaryFile(const char *filename, void* data, size_t cnt, bool append = false);
bool tdWriteTextFile(const char *filename, char *data, size_t cnt, bool append = false);
bool tdDeleteFile(const char *filename);
void tdGetFiles(TdMemoryArena &mem_arena, const char *path, TdArray<char *> &files, uint32 r_trim = 0);
ALWAYS_INLINE bool tdMouseIsLeftClick(const TdMouseState& mouse) { return !mouse.mb_left.button_ended_down && mouse.mb_left.half_transition_count; }
ALWAYS_INLINE bool tdMouseIsLeftPress(const TdMouseState& mouse) { return mouse.mb_left.button_ended_down && mouse.mb_left.half_transition_count; }
ALWAYS_INLINE bool tdMouseIsRightClick(const TdMouseState& mouse) { return !mouse.mb_right.button_ended_down && mouse.mb_right.half_transition_count; }
ALWAYS_INLINE bool tdMouseIsRightPress(const TdMouseState& mouse) { return mouse.mb_left.button_ended_down && mouse.mb_left.half_transition_count; }
ALWAYS_INLINE bool tdKeyboardIsKeyPressed(const TdKeyboardState& keyboard, int32 key) { return keyboard.key_state[key] > 0; }
ALWAYS_INLINE bool tdKeyboardIsKeyReleased(const TdKeyboardState& keyboard, int32 key) { return !keyboard.key_state[key]; }
ALWAYS_INLINE bool tdKeyboardIsKeyPressedNew(const TdKeyboardState& keyboard, int32 key) { return keyboard.key_state[key] > 0 && !keyboard.prev_state[key]; }
ALWAYS_INLINE bool tdKeyboardIsKeyReleasedNew(const TdKeyboardState& keyboard, int32 key) { return !keyboard.key_state[key] && keyboard.prev_state[key] > 0; }
float tdWrapRadian(float angle);
void tdVector2MinMax(Vector2&, Vector2&);
void tdVector3MinMax(Vector3&, Vector3&);
void tdSetMinMax(TdPoint3 & min, TdPoint3 & max);
void tdClamp(TdPoint2 & p, const TdPoint2 &min, const TdPoint2 &max);
void tdClamp(TdPoint3 & p, const TdPoint3 &min, const TdPoint3 &max);
void tdClamp(Vector2 & p, const Vector2 &min, const Vector2 &max);
void tdClamp(Vector3 & p, const Vector3 &min, const Vector3 &max);
int64 tdHexToInt(const char *hex);

ALWAYS_INLINE float tdAngleFromDir(Vector2 dir) // dir is not normalized
{
	Vector2 norm_dir = normalize(dir);
	float angle = -atan2(norm_dir.y, norm_dir.x) - M_PI_2;
	return angle;
}

ALWAYS_INLINE float tdAngleFromNormDir(Vector2 norm_dir)
{
	float angle = -atan2(norm_dir.y, norm_dir.x) - M_PI_2;
	return angle;
}

ALWAYS_INLINE Vector2 tdRotate(Vector2 pos, Vector2 about, float angle)
{
	//x0+(x−x0)cosϕ+(y−y0)sinϕ
	//y0−(x−x0)sinϕ+(y−y0)cosϕ
	float cos_a = cos(angle);
	float sin_a = sin(angle);
	float cx = pos.x - about.x;
	float cy = pos.y - about.y;
	Vector2 result;
	result.x = about.x + cx * cos_a + cy * sin_a;
	result.y = about.y - cx * sin_a + cy * cos_a;
	return result;
}

ALWAYS_INLINE float tdLerp(float value1, float value2, float amount)
{
	return value1 + (value2 - value1) * amount;
}

ALWAYS_INLINE Vector2 tdLerp(Vector2 value1, Vector2 value2, float amount)
{
	Vector2 result;
	result.x = value1.x + (value2.x - value1.x) * amount;
	result.y = value1.y + (value2.y - value1.y) * amount;
	return result;
}

ALWAYS_INLINE Vector3 tdLerp(Vector3 value1, Vector3 value2, float amount)
{
	Vector3 result;
	result.x = value1.x + (value2.x - value1.x) * amount;
	result.y = value1.y + (value2.y - value1.y) * amount;
	result.z = value1.z + (value2.z - value1.z) * amount;
	return result;
}

ALWAYS_INLINE Vector3 tdBoundingBoxCenter(const TdBoundingBox& box) { return (box.max - box.min) * 0.5f + box.min; }
ALWAYS_INLINE TdPoint2 tdRectCenter(const TdRect& outer, int w = 0, int h = 0)
{
	TdPoint2 r;
	r.x = (outer.w - w) / 2 + outer.x;
	r.y = (outer.h - h) / 2 + outer.y;
	return r;
}

ALWAYS_INLINE TdPoint2 tdRectCenter(const TdRect& outer, const TdRect& inner)
{
	TdPoint2 r;
	r.x = (outer.w - inner.w) / 2 + outer.x;
	r.y = (outer.h - inner.h) / 2 + outer.y;
	return r;
}

ALWAYS_INLINE TdPoint2 tdRectCenter(const VkViewport& outer, const TdRect& inner)
{
	TdPoint2 r;
	r.x = (outer.width - inner.w) / 2 + outer.x;
	r.y = (outer.height - inner.h) / 2 + outer.y;
	return r;
}

ALWAYS_INLINE TdRect tdRectCenter(const VkViewport& outer, int w, int h)
{
	TdRect r;
	r.x = (outer.width - w) / 2 + outer.x;
	r.y = (outer.height - h) / 2 + outer.y;
	r.w = w;
	r.h = h;
	return r;
}

ALWAYS_INLINE int16 tdClamp(int16 val, int16 min, int16 max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE uint16 tdClamp(uint16 val, uint16 min, uint16 max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE int32 tdClamp(int32 val, int32 min, int32 max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE uint32 tdClamp(uint32 val, uint32 min, uint32 max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE int64 tdClamp(int64 val, int64 min, int64 max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE uint64 tdClamp(uint64 val, uint64 min, uint64 max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE float tdClamp(float val, float min, float max)
{
	return val < min ? min : val > max ? max : val;
}

ALWAYS_INLINE double tdClamp(double val, double min, double max)
{
	return val < min ? min : val > max ? max : val;
}

bool tdRayBoxIntersect(const TdRay&, const TdBoundingBox&, double t);

ALWAYS_INLINE uint32_t tdUpperPowerOfTwo(uint32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

ALWAYS_INLINE uint64_t tdUpperPowerOfTwo(uint64 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

ALWAYS_INLINE uint32_t tdIntegerHash(uint32 h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}

ALWAYS_INLINE uint64_t tdIntegerHash(uint64 k)
{
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;
	return k;
}

ALWAYS_INLINE bool tdIntersect(Vector2 p, TdRect r)
{
	return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
}

ALWAYS_INLINE bool tdIntersect(TdPoint2 p, TdRect r)
{
	return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
}

ALWAYS_INLINE bool tdIntersect(TdRect r1, TdRect r2)
{
	return r2.x < r1.x + r1.w && r2.y < r1.y + r1.h &&
		   r2.x + r2.w > r1.x && r2.y + r2.h > r1.y;
}

ALWAYS_INLINE bool tdIntersect(Vector4 r1, Vector4 r2)
{
	return r2.x < r1.x + r1.z && r2.y < r1.y + r1.w &&
		   r2.x + r2.z > r1.x && r2.y + r2.w > r1.y;
}

ALWAYS_INLINE bool tdIntersectExtent(Vector4 r1, Vector4 r2)
{
	return r2.x <= r1.z && r2.y <= r1.w && r2.z >= r1.x && r2.w >= r1.y;
}

ALWAYS_INLINE bool tdIntersectExtent(Vector2 p, Vector4 r)
{
	return p.x >= r.x && p.x <= r.z && p.y >= r.y && p.y <= r.w;
}

template<size_t size>
void strcpy_safe(char (&dest)[size], const char* src)
{
	for (size_t i = 0; i < size; ++i)
	{
		dest[i] = *src;
		if (!(*src)) return;
		++src;
	}
    dest[size - 1] = 0;
}

WCHAR* swprintf_comma_core(WCHAR* buff, int len);
char* sprintf_comma_core(char* buff, int len);

ALWAYS_INLINE WCHAR* swprintf_comma(int32 v, WCHAR* buff)
{
	int len = swprintf(buff, 16, L"%d", v);
	return swprintf_comma_core(buff, len);
}

ALWAYS_INLINE WCHAR* swprintf_comma(uint32 v, WCHAR* buff)
{
	int len = swprintf(buff, 16, L"%u", v);
	return swprintf_comma_core(buff, len);
}

ALWAYS_INLINE WCHAR* swprintf_comma(int64 v, WCHAR* buff)
{
	int len = swprintf(buff, 16, L"%lld", v);
	return swprintf_comma_core(buff, len);
}

ALWAYS_INLINE WCHAR* swprintf_comma(uint64 v, WCHAR* buff)
{
	int len = swprintf(buff, 16, L"%llu", v);
	return swprintf_comma_core(buff, len);
}

ALWAYS_INLINE char* sprintf_comma(int32 v, char* buff)
{
	int len = sprintf(buff, "%d", v);
	return sprintf_comma_core(buff, len);
}

ALWAYS_INLINE char* sprintf_comma(uint32 v, char* buff)
{
	int len = sprintf(buff, "%u", v);
	return sprintf_comma_core(buff, len);
}

ALWAYS_INLINE char* sprintf_comma(int64 v, char* buff)
{
	int len = sprintf(buff, "%lld", v);
	return sprintf_comma_core(buff, len);
}

ALWAYS_INLINE char* sprintf_comma(uint64 v, char* buff)
{
	int len = sprintf(buff, "%llu", v);
	return sprintf_comma_core(buff, len);
}

template <typename T>
ALWAYS_INLINE void WriteBin(uint8** out, const T& data)
{
	memcpy(*out, &data, sizeof(T));
	*out += sizeof(T);
}

template <typename T>
ALWAYS_INLINE void WriteBinArray(uint8** out, const T* data, uint32 count)
{
	WriteBin(out, count);
	memcpy(*out, data, sizeof(T) * count);
	*out += sizeof(T) * count;
}

template <typename T>
ALWAYS_INLINE void WriteBinPtr(uint8** out, const T base, const T data)
{
	if (data == nullptr)
		WriteBin(out, (int32)-1);
	else
		WriteBin(out, (int32)(data - base));
}

template <typename T>
ALWAYS_INLINE T& ReadBin(uint8** in)
{
	uint8* in1 = *in;
	*in += sizeof(T);
	return (T&)(*in1);
}

template <typename T>
ALWAYS_INLINE void ReadBin(uint8** in, T& t)
{
	memcpy((void*)&t, *in, sizeof(T));
	*in += sizeof(T);
}

template <typename T>
ALWAYS_INLINE uint32 ReadBinArray(uint8** in, const T* data)
{
	uint32 count;
	ReadBin(in, count);
	memcpy((void*)data, *in, sizeof(T) * count);
	*in += sizeof(T) * count;
	return count;
}

template <typename T>
ALWAYS_INLINE T ReadBinPtr(uint8** in, const T base)
{
	int32 i = ReadBin<int32>(in);
	return i < 0 ? nullptr : base + i;
}

template <typename T>
ALWAYS_INLINE void ReadBinPtr(uint8** in, const T base, T& data)
{
	int32 i = ReadBin<int32>(in);
	data = i < 0 ? nullptr : base + i;
}

}