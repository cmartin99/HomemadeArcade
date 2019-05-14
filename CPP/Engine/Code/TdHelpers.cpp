#include "TdHelpers.h"

using namespace glm;

namespace eng {

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
char* tdLastErrorAsString(char* result_buffer, size_t result_buffer_len, bool concatenate)
{
	assert(result_buffer);

	DWORD errorID = ::GetLastError();
	if(errorID == 0)
	{
		result_buffer[0] = 0;
		return result_buffer;
	}

	LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	if (concatenate)
		strncat(result_buffer, messageBuffer, min(result_buffer_len, size));
	else
		strncpy(result_buffer, messageBuffer, min(result_buffer_len, size));

	result_buffer[result_buffer_len - 1] = 0;
    LocalFree(messageBuffer);
    return result_buffer;
}

float tdWrapRadian(float angle)
{
	const float _pi = pi<float>();
	const float pi2 = two_pi<float>();
	while (angle < -_pi) angle += pi2;
	while (angle > _pi) angle -= pi2;
	return angle;
}

void tdVector2MinMax(Vector2& v1, Vector2& v2)
{
	Vector2 min;
	Vector2 max;

	if (v1.x < v2.x)
	{
		min.x = v1.x;
		max.x = v2.x;
	}
	else
	{
		min.x = v2.x;
		max.x = v1.x;
	}

	if (v1.y < v2.y)
	{
		min.y = v1.y;
		max.y = v2.y;
	}
	else
	{
		min.y = v2.y;
		max.y = v1.y;
	}

	v1.x = min.x;
	v1.y = min.y;

	v2.x = max.x;
	v2.y = max.y;
}

void tdVector3MinMax(Vector3& v1, Vector3& v2)
{
	Vector3 min;
	Vector3 max;

	if (v1.x < v2.x)
	{
		min.x = v1.x;
		max.x = v2.x;
	}
	else
	{
		min.x = v2.x;
		max.x = v1.x;
	}

	if (v1.y < v2.y)
	{
		min.y = v1.y;
		max.y = v2.y;
	}
	else
	{
		min.y = v2.y;
		max.y = v1.y;
	}

	if (v1.z < v2.z)
	{
		min.z = v1.z;
		max.z = v2.z;
	}
	else
	{
		min.z = v2.z;
		max.z = v1.z;
	}

	v1.x = min.x;
	v1.y = min.y;
	v1.z = min.z;

	v2.x = max.x;
	v2.y = max.y;
	v2.z = max.z;
}

void tdSetMinMax(TdPoint3& min, TdPoint3& max)
{
	if (max.x < min.x)
	{
		int s = min.x;
		min.x = max.x;
		max.x = s;
	}
	if (max.y < min.y)
	{
		int s = min.y;
		min.y = max.y;
		max.y = s;
	}
	if (max.z < min.z)
	{
		int s = min.z;
		min.z = max.z;
		max.z = s;
	}
}

void tdClamp(TdPoint2& p, const TdPoint2& min, const TdPoint2& max)
{
	if (p.x < min.x) p.x = min.x;
	else if (p.x > max.x) p.x = max.x;
	if (p.y < min.y) p.y = min.y;
	else if (p.y > max.y) p.y = max.y;
}

void tdClamp(TdPoint3& p, const TdPoint3& min, const TdPoint3& max)
{
	if (p.x < min.x) p.x = min.x;
	else if (p.x > max.x) p.x = max.x;
	if (p.y < min.y) p.y = min.y;
	else if (p.y > max.y) p.y = max.y;
	if (p.z < min.z) p.z = min.z;
	else if (p.z > max.z) p.z = max.z;
}

void tdClamp(Vector2& p, const Vector2& min, const Vector2& max)
{
	if (p.x < min.x) p.x = min.x;
	else if (p.x > max.x) p.x = max.x;
	if (p.y < min.y) p.y = min.y;
	else if (p.y > max.y) p.y = max.y;
}

void tdClamp(Vector3& p, const Vector3& min, const Vector3& max)
{
	if (p.x < min.x) p.x = min.x;
	else if (p.x > max.x) p.x = max.x;
	if (p.y < min.y) p.y = min.y;
	else if (p.y > max.y) p.y = max.y;
	if (p.z < min.z) p.z = min.z;
	else if (p.z > max.z) p.z = max.z;
}

WCHAR* swprintf_comma_core(WCHAR* buff, int len)
{
	for (int i = len - 3; i > 0; i -= 3)
	{
		memmove(buff + i + 1, buff + i, sizeof(WCHAR) * (len - i + 2));
		buff[i] = L',';
	}
	return buff;
}

char* sprintf_comma_core(char* buff, int len)
{
	for (int i = len - 3; i > 0; i -= 3)
	{
		memmove(buff + i + 1, buff + i, len - i + 2);
		buff[i] = ',';
	}
	return buff;
}

_off_t tdGetFileSize(const char *filename)
{
	struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

char* tdReadAllTextFileAndNullTerminate(const char* filename)
{
    FILE *fp = fopen(filename, "rb+");
    if (!fp) return nullptr;

	fseek(fp, 0L, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char* data = (char *)malloc(size + 1);
	size_t retval = fread(data, size, 1, fp);
    data[size] = 0;
    fclose(fp);
	//assert(retval == 1);

    return data;
}

char* tdReadBinaryFile(const char *filename, TdMemoryArena& arena, size_t* psize)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) return nullptr;

	fseek(fp, 0L, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char* data = tdMalloc<char>(arena, size);
	size_t retval = fread(data, size, 1, fp);
	fclose(fp);

	assert(retval == 1);
	*psize = size;

	return data;
}

char* tdReadBinaryFile(const char *filename, size_t* psize)
{
	long int size;
	size_t retval;
	void *data;

	FILE *fp = fopen(filename, "rb");
	if (!fp) return nullptr;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	data = malloc(size);
	retval = fread(data, size, 1, fp);
	fclose(fp);

	assert(retval == 1);
	*psize = size;

	return (char*)data;
}

bool tdWriteBinaryFile(const char *filename, void* data, size_t cnt, bool append)
{
	FILE *fp = fopen(filename, append ? "ab" : "wb");
	if (!fp) return false;

	fwrite(data, sizeof(uint8), cnt, fp);
	fclose(fp);

	return true;
}

bool tdWriteTextFile(const char *filename, char* data, size_t cnt, bool append)
{
	FILE *fp = fopen(filename, append ? "a" : "w");
	if (!fp) return false;

	fwrite(data, sizeof(char), cnt, fp);
	fclose(fp);

	return true;
}

bool tdDeleteFile(const char *filename)
{
	return DeleteFile(filename) != 0;
}

bool tdRayBoxIntersect(const TdRay& ray, const TdBoundingBox& box, double t)
{
  // This is actually correct, even though it appears not to handle edge cases
  // (ray.n.{x,y,z} == 0).  It works because the infinities that result from
  // dividing by zero will still behave correctly in the comparisons.  Rays
  // which are parallel to an axis and outside the box will have tmin == inf
  // or tmax == -inf, while rays inside the box will have tmin and tmax
  // unchanged.

  TdRay opt_ray = { ray.orig, Vector3(1.0/ray.dir.x, 1.0/ray.dir.y, 1.0/ray.dir.z) };

  double tx1 = (box.min.x - opt_ray.orig.x)*opt_ray.dir.x;
  double tx2 = (box.max.x - opt_ray.orig.x)*opt_ray.dir.x;

  double tmin = min<double>(tx1, tx2);
  double tmax = max<double>(tx1, tx2);

  double ty1 = (box.min.y - opt_ray.orig.y)*opt_ray.dir.y;
  double ty2 = (box.max.y - opt_ray.orig.y)*opt_ray.dir.y;

  tmin = max<double>(tmin, min<double>(ty1, ty2));
  tmax = min<double>(tmax, max<double>(ty1, ty2));

  double tz1 = (box.min.z - opt_ray.orig.z)*opt_ray.dir.z;
  double tz2 = (box.max.z - opt_ray.orig.z)*opt_ray.dir.z;

  tmin = max<double>(tmin, min<double>(tz1, tz2));
  tmax = min<double>(tmax, max<double>(tz1, tz2));

  return tmax >= max<double>(0.0, tmin) && tmin < t;
}

void tdGetFiles(TdMemoryArena& mem_arena, const char* path, TdArray<char*>& files, uint32 r_trim)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(path, &ffd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
	   //DisplayErrorBox(TEXT("FindFirstFile"));
	   return;
	}

	do
	{
	   if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	   {
		   int len = strlen(ffd.cFileName) - r_trim;
		   char* t = tdMalloc<char>(mem_arena, len + 1);
		   strncpy(t, ffd.cFileName, len);
		   t[len] = 0;
		   tdArrayAdd(files, t);
	   }
	}
	while (FindNextFile(hFind, &ffd) != 0);
}

static const long hextable[] = {
	// bit aligned access into this table is considerably faster for most modern processors
	// for the space conscious, reduce to signed char.
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1, 0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

int64 tdHexToInt(const char *hex)
 {
	int64 ret = 0;
	while (*hex && ret >= 0)
		ret = (ret << 4) | hextable[*hex++];
	return ret;
}

}