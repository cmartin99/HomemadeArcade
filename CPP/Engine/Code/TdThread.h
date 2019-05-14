#pragma once
#include "TdDataTypes.h"

namespace eng {

struct TdThreadQueue;

struct TdThreadInfo
{
	TdThreadQueue* queue;
	uint32 thread_id;
	DWORD platform_thread_id;
};

typedef void (*TdThreadCallBack)(TdThreadInfo&, void* data);

struct TdThreadWorkItem
{
	TdThreadCallBack callback;
	void* data;
};

struct TdThreadInfo;

struct TdThreadQueue
{
	LONG volatile next_read;
	LONG volatile next_write;
	LONG volatile completed_count;
	LONG volatile completed_goal;
	CRITICAL_SECTION critical_section;
	HANDLE semaphore;
	HANDLE mutex;
	LONG item_capacity;
	TdThreadWorkItem* items;
	TdThreadInfo* workers;
};

void TdThreadManualQueueUpdate(TdThreadQueue&);
void TdThreadAddJob(TdThreadQueue&, TdThreadCallBack, void* data);
void TdThreadWaitAllComplete(TdThreadQueue&);
void TdInitThreadQueue(TdThreadQueue& queue, uint32 item_capacity, uint32 worker_count, uint64 affinity = 0);

}