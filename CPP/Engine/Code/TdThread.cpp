#include "TdThread.h"

namespace eng {
	
bool ExecuteNextQueueItem(TdThreadInfo& info)
{
	assert(info.queue);
	bool result = false;
	TdThreadQueue& queue = *info.queue;
	
	EnterCriticalSection(&queue.critical_section);
	{
		LONG orig_next_read = queue.next_read;
		if (orig_next_read != queue.next_write)
		{
			LONG new_next_read = queue.next_read + 1;
			if (new_next_read >= queue.item_capacity) 
				new_next_read = 0;
			TdThreadWorkItem* item = &queue.items[orig_next_read];
			item->callback(info, item->data);
			queue.next_read = new_next_read;
			++queue.completed_count;
			result = true;
		}
	}
	LeaveCriticalSection(&queue.critical_section);
	return result;
}

void TdThreadAddJob(TdThreadQueue& queue, TdThreadCallBack callback, void* data)
{
	assert(callback);
	
	EnterCriticalSection(&queue.critical_section);
	{
		LONG orig_next_write = queue.next_write;
		LONG new_next_write = queue.next_write + 1;
		if (new_next_write >= queue.item_capacity) 
			new_next_write = 0;
		assert(new_next_write != queue.next_read);	
		queue.items[orig_next_write].callback = callback;
		queue.items[orig_next_write].data = data;
		queue.next_write = new_next_write;
		++queue.completed_goal;
	}
	LeaveCriticalSection(&queue.critical_section);
	if (queue.workers) ReleaseSemaphore(queue.semaphore, 1, NULL);
}

void TdThreadWaitAllComplete(TdThreadQueue& queue)
{
	while (queue.completed_count < queue.completed_goal);
	
	//queue.completed_count = 0;
	//queue.completed_goal = 0;
}

void TdThreadManualQueueUpdate(TdThreadQueue& queue)
{
	TdThreadInfo info = { &queue, 0, 0 };
	while (ExecuteNextQueueItem(info));	
}

DWORD WINAPI ThreadProc(LPVOID lp)
{
	TdThreadInfo* info = (TdThreadInfo*)lp;
	
	for (;;)
	{
		while (ExecuteNextQueueItem(*info));
		WaitForSingleObjectEx(info->queue->semaphore, INFINITE, false);
	}
}

void TdInitThreadQueue(TdThreadQueue& queue, uint32 item_capacity, uint32 worker_count)
{
	memset(&queue, 0, sizeof(TdThreadQueue));

	queue.item_capacity = item_capacity;
	queue.items = tdMalloc<TdThreadWorkItem>(eng_arena, item_capacity);
	memset(queue.items, 0, sizeof(TdThreadWorkItem) * item_capacity);

	InitializeCriticalSection(&queue.critical_section);
	//queue.mutex = CreateMutex(0, false, 0);

	if (worker_count) // Don't create threads if worker_count is zero (threads turned off)
	{
		queue.semaphore = CreateSemaphoreEx(0, 0, worker_count, NULL, 0, SEMAPHORE_ALL_ACCESS);
		queue.workers = tdMalloc<TdThreadInfo>(eng_arena, worker_count);
		memset(queue.workers, 0, sizeof(TdThreadInfo) * worker_count);
		
		for (uint32 i = 0; i < worker_count; ++i)
		{
			queue.workers[i].queue = &queue;
			queue.workers[i].thread_id = i;
			HANDLE handle = CreateThread(0, 0, ThreadProc, queue.workers + i, 0, &queue.workers[i].platform_thread_id);
			CloseHandle(handle); 
		}
	}
}

}