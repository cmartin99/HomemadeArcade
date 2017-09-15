#include "TdPool.h"

namespace eng {

uint64 TdPoolInt::GetNext()
{
	if (next == items.count)
	{
		tdArrayAdd(items, next);
		if (next == 0) tdArrayAdd(items,++ next);
		return next++;
	}

	uint64 result = items[next];
	if (!result) 
	{
		tdArraySetValue(items, next, next);
		return next++;
	}

	while (true)
	{
		if (next == items.cap)
		{
			tdArrayAdd(items, next);
			return next++;
		}

		result = items[next];
		if (!result) 
		{
			tdArraySetValue(items, next, next);
			return next++;
		}

		++next;
	}
}

void TdPoolInt::Release(uint64 id)
{
	if (id < items.count)
	{
		tdArraySetValue(items, id, (uint64)0);
		if (id < next) next = id;
	}
}

bool TdPoolHandle::IsExists(uint64 id) const
{ 
	uint32 i = id & 0xffffffff;
	return (i < items.count ? (items[i] << 32) != 0 : false);
}

uint64 TdPoolHandle::GetNext()
{
	if (next == items.count)
	{
		tdArrayAdd(items, next);
		if (next == 0) tdArrayAdd(items, ++next);
		return next++;
	}

	uint64 result = items[next];
	uint32 i = result & 0xffffffff;
	if (!i)
	{
		uint64 v = ((result >> 32) + 1) << 32;
		tdArraySetValue(items, next, next + v);
		return next++;
	}

	while (true)
	{
		if (next == items.cap)
		{
			tdArrayAdd(items, next);
			return next++;
		}

		result = items[next];
		i = result & 0xffffffff;
		if (!i)
		{
			uint64 v = ((result >> 32) + 1) << 32;
			tdArraySetValue(items, next, next + v);
			return next++;
		}

		++next;
	}
}

void TdPoolHandle::Release(uint64 id)
{
	uint32 i = id & 0xffffffff;
	if (i < items.count)
	{
		tdArraySetValue(items, i, id & 0xffffffff00000000L);
		if (i < next) next = i;
	}
}

}