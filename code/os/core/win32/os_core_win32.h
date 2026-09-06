
#ifndef OS_CORE_WIN32_H
#define OS_CORE_WIN32_H

////////////////////////////////
//~ rjf: Entity Types

typedef enum OS_W32_EntityKind
{
	OS_W32_EntityKind_Null,
	OS_W32_EntityKind_Thread,
	OS_W32_EntityKind_Mutex,
	OS_W32_EntityKind_RWMutex,
	OS_W32_EntityKind_ConditionVariable,
}
OS_W32_EntityKind;

typedef struct OS_W32_Entity OS_W32_Entity;
struct OS_W32_Entity
{
	OS_W32_Entity *next;
	OS_W32_EntityKind kind;
	union
	{
		struct
		{
			OS_ThreadFunctionType *func;
			void *ptr;
			HANDLE handle;
			DWORD tid;
		} thread;
		CRITICAL_SECTION mutex;
		SRWLOCK rw_mutex;
		CONDITION_VARIABLE cv;
	};
};

struct OS_W32_State
{
	OS_SystemInfo system_info;
	U64 microsecond_resolution;
	B32 granular_sleep_enabled;
	
	// rjf: entity storage
	CRITICAL_SECTION entity_mutex;
	Arena *entity_arena;
	OS_W32_Entity *entity_free;
};

struct OS_W32_FileIter
{
	HANDLE handle;
	WIN32_FIND_DATAA find_data;
};
StaticAssert(sizeof(OS_W32_FileIter) <= sizeof(Member(OS_FileIter, memory)), file_iter_memory_size);

global OS_W32_State os_w32_state = {0};

////////////////////////////////
//~ rjf: @os_hooks Time (Implemented Per-OS)

internal U64 os_now_microseconds(void);
internal void os_sleep_milliseconds(U32 msec);
internal void os_wait(U64 end_time_us);
internal u64 os_now_unix();
internal DateTime os_local_time();
internal r64 os_time_diff(u64 end, u64 start);
internal DateTime os_local_time_from_timestamp(u64 timestamp);

////////////////////////////////
//~ rjf: Time Conversion Helpers
internal void os_w32_system_time_from_date_time(SYSTEMTIME *out, DateTime *in);
internal void os_w32_date_time_from_system_time(DateTime *out, SYSTEMTIME *in);
internal DenseTime os_w32_dense_time(void);
internal void os_w32_dense_time_from_file_time(DenseTime *out, FILETIME *in);

internal OS_FileIter *os_file_iter_begin(Arena *arena, String8 path, OS_FileIterFlags flags);
internal b32 os_file_iter_next(Arena *arena, OS_FileIter *iter, OS_FileInfo *info);
internal void os_file_iter_end(OS_FileIter *it);

#endif //OS_CORE_WIN32_H
