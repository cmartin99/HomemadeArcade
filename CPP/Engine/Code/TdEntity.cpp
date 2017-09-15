#include "TdEntity.h"

namespace eng {

static TdPoolHandle entity_ids;

void TdInitEntitySystem(uint32 entity_pool_count)
{
	entity_ids.Init(entity_pool_count);
}

inline uint32 HandleIndex(TdEntity e)
{
	return e.handle & 0xffffffff;
}

inline uint32 HandleIndex(uint64 e)
{
	return e & 0xffffffff;
}

bool IsEntityAlive(TdEntity e)
{
	if (e.handle)
	{
		uint64 h = entity_ids[HandleIndex(e)];
		uint32 i = HandleIndex(h);
		if (i)
		{
			uint32 v1 = e.handle >> 32;
			uint32 v2 = h >> 32;
			return v1 == v2;
		}
	}
	return false;
}

TdEntity tdEntityCreate()
{
	TdEntity e = { entity_ids.GetNext() };
	return e;
}

void tdEntityAssign(TdEntity e, uint64 type, void* com_ptr)
{
}

void tdEntityDelete(TdEntity e)
{
	if (IsEntityAlive(e))
	{
		entity_ids.Release(e.handle);
	}
}

//void tdEntityAssign<Com>(TdEntity e, Com& c)
//{
//}

}