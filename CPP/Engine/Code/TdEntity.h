#pragma once
#include "TdBase.h"

namespace eng
{

struct TdEntity
{
	uint64 handle;
	bool operator==(TdEntity& e) const { return handle == e.handle; }
};

TdEntity tdEntityCreate();
void tdEntityDelete(TdEntity);
void tdEntityAssign(TdEntity, uint64 type, void* com_ptr);
//void TdEntityAssign<T>(TdEntity, Com&);

}