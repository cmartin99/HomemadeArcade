#pragma once
#include "TdBase.h"

namespace eng {

template <typename T>
struct TdArray
{
	uint32 count;
	uint32 cap;
	T* ptr;

	T& operator[](uint32 i) { assert(i < count); return ptr[i]; }
	const T& operator[](uint32 i) const { assert(i < count); return ptr[i]; }
};

template<typename T>
void tdArrayInit(TdArray<T>& a, uint32 capacity, bool zero_mem = false)
{
	tdArrayInit(a, eng_arena, capacity, zero_mem);
}

template<typename T>
void tdArrayInit(TdArray<T>& a, TdMemoryArena& arena, uint32 capacity, bool zero_mem = false)
{
	a.ptr = tdMalloc<T>(arena, capacity);
	a.cap = capacity;
	a.count = 0;
	if (zero_mem) memset(a.ptr, 0, sizeof(T) * capacity);
}

template<typename T>
uint32 tdArrayGetIndex(TdArray<T>& a, T t)
{
	for (uint32 i = 0; i < a.count; ++i)
	{
		if (a.ptr[i] == t) return i;
	}
	return UINT32_MAX;
}

template<typename T>
void tdArrayAdd(TdArray<T>& a, T t)
{
	assert(a.ptr);
	assert(a.count < a.cap);
	a.ptr[a.count++] = t;
}

template<typename T>
void tdArrayInsert(TdArray<T>& a, T t, uint32 i)
{
	assert(a.ptr);
	assert(a.count < a.cap);
	assert(i >= 0 && i < a.count);
	memmove(a.ptr + i, a.ptr + i + 1, sizeof(T));
	a.ptr[i] = t;
	++a.count;
}

template<typename T>
void tdArrayRemove(TdArray<T>& a, uint32 i, uint32 count = 1)
{
	assert(i >= 0);
	assert(count > 0);

	if (a.count > 0 && i < a.count)
	{
		if (i + count >= a.count)
		{
			a.count = i;
		}
		else
		{
			memmove(a.ptr + i, a.ptr + i + count, (a.count - i - count) * sizeof(T));
			a.count -= count;
		}
	}
}

template<typename T>
void tdArrayRemoveRef(TdArray<T>& a, T t)
{
	uint32 i = tdArrayGetIndex(a, t);
	if (i != UINT32_MAX) tdArrayRemove(a, i);
}

template<typename T>
T* tdArrayPush(TdArray<T>& a)
{
	assert(a.ptr);
	assert(a.count < a.cap);
	T* result = a.ptr + a.count;
	++a.count;
	return result;
}

template<typename T>
void tdArraySetValue(TdArray<T>& a, uint32 i, T val)
{
	assert(a.ptr);
	assert(i < a.count);
	a.ptr[i] = val;
}

template<typename T>
T tdArrayPeek(TdArray<T>& a)
{
	assert(a.ptr);
	assert(a.count > 0);
	return a.ptr[a.count - 1];
}

template<typename T>
T tdArrayPop(TdArray<T>& a)
{
	assert(a.ptr);
	assert(a.count > 0);
	return a.ptr[--a.count];
}

template<typename T>
void tdArrayClear(TdArray<T>& a)
{
	a.count = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct TdArray1
{
	TdArray1() : capacity(0), count(0), data(0)  { Reserve(10); }
	TdArray1(uint64 reserve, bool clear_on_allocate = false) : capacity(0), count(0), data(0) { Reserve(reserve, clear_on_allocate); }
	~TdArray1() { if (data) free(data); }
	void Add(T v);
	void Add(TdArray1<T>&);
	void Push(T v) { Add(v); }
	void PushCount(int inc = 1);
	void Remove(T v);
	void RemoveAt(uint64 i);
	void Clear() { count = 0; }
	uint64 GetCapacity() const { return capacity & 0x7fffffffffffffff; }
	uint64 GetCount() const { return count; }
	uint64 GetIndex(T v) const;
	T* ptr() const { return data; }
	T* ptr(uint64 i) const { return data + i; }
	T GetValue(uint64 i) const { assert(i >= 0 && i < count); return data[i]; }
	T GetItem(uint64 i) const { assert(i >= 0 && i < count); return data[i]; }
	void SetValue(uint64 i, T v);
	T Pop() { return data[--count]; }
	T Peek() const { return data[count - 1]; }
	void Reserve(uint64 size, bool zero_mem = false, bool pad_out_cap = false);
	void Free() { if (data) free(data); data = nullptr; }
	void SetClearOnAllocate(bool  clear) { capacity |= ((uint64)1 << 63); }

	const T& operator[](uint64 i) const
	{
		uint64 cap = capacity & 0x7fffffffffffffff;
		assert(i >= 0 && i < cap);
		return *(data + i);
	}

	T& operator[](uint64 i)
	{
		uint64 cap = capacity & 0x7fffffffffffffff;
		assert(i >= 0 && i < cap);
		return *(data + i);
	}

private:
	uint64 capacity;
	uint64 count;
	T* data;
};

template<typename T>
void TdArray1<T>::SetValue(uint64 i, T v)
{
	uint64 cap = capacity & 0x7fffffffffffffff;
	if (i >= cap) Reserve(i + cap / 2, (capacity >> 63) > 0);
	data[i] = v;
}

template<typename T>
void TdArray1<T>::Add(T v)
{
	uint64 cap = capacity & 0x7fffffffffffffff;
	if (count == cap) Reserve(cap + cap / 2, (capacity >> 63) > 0);
	data[count++] = v;
}

template<typename T>
void TdArray1<T>::Add(TdArray1<T>& src)
{
	if (src.count)
	{
		uint64 pre_count = count;
		PushCount(src.count);
		memmove(data + pre_count, src.data, src.count * sizeof(T));
	}
}

template<typename T>
void TdArray1<T>::PushCount(int inc = 1)
{
	uint64 cap = capacity & 0x7fffffffffffffff;
	if (count + inc >= cap) Reserve(cap + inc + cap / 2, (capacity >> 63) > 0);
	count += inc;
}

template<typename T>
void TdArray1<T>::Reserve(uint64 size, bool zero_mem, bool pad_out_cap)
{
	uint64 cap = capacity & 0x7fffffffffffffff;
	if (pad_out_cap) size += cap / 2;

	if (cap < size)
	{
		uint64 old_cap = cap;
		capacity = size;
		if (zero_mem) capacity |= ((uint64)1 << 63);

		T* newData = (T*)malloc(sizeof(T) * size);
		if (data)
		{
			memmove(newData, data, sizeof(T) * count);
			free(data);
		}

		if (zero_mem)
		{
			memset(newData + old_cap, 0, sizeof(T) * (size - old_cap));
		}

		data = newData;
	}
}

template<typename T>
uint64 TdArray1<T>::GetIndex(T v) const
{
	for (uint64 i = 0; i < count; ++i)
	{
		if (data[i] == v) return i;
	}
	return UINT64_MAX;
}

template<typename T>
void TdArray1<T>::Remove(T v)
{
	RemoveAt(GetIndex(v));
}

template<typename T>
void TdArray1<T>::RemoveAt(uint64 i)
{
	if (count > 0 && i >= 0 && i < count)
	{
		if (i == count - 1)
		{
			--count;
		}
		else
		{
			if (count > 1)
			{
				memmove(data + i, data + i + 1, (count - 1 - i) * sizeof(T));
			}
			--count;
		}
	}
}

}