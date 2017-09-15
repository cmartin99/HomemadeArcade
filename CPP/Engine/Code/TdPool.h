#pragma once
#include "TdArray.h"

namespace eng {

struct TdPoolInt
{
	uint64 next;
	TdArray<uint64> items;
	TdPoolInt() {}
	TdPoolInt(int reserve) { Init(reserve); }
	void Init(int reserve) { tdArrayInit(items, reserve, true); next = 0; }
	uint64 GetNext();
	uint64 GetCount() const { return items.count; }
	uint64* ptr() const { return items.ptr; }
	void Release(uint64 id);
	bool IsExists(uint64 id) const { return (id < items.count ? items[id] != 0 : false); }
};

struct TdPoolHandle
{
	uint64 next;
	TdArray<uint64> items;
	TdPoolHandle() {}
	TdPoolHandle(int reserve) { Init(reserve); }
	void Init(int reserve) { tdArrayInit(items, reserve, true); next = 0; }
	uint64 GetNext();
	uint64 GetCount() const { return items.count; }
	uint64 *ptr() const { return items.ptr; }
	void Release(uint64 id);
	bool IsExists(uint64 id) const;
	const uint64& operator[](uint32 i) const { return items[i]; }
	uint64 operator[](uint32 i) { return items[i]; }
};

}