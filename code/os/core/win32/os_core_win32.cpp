
//
//- rsb: registry calls
//

internal b32
w32_hkcu_delete(String8 path, String8 key)
{
	HKEY handle;
	LONG hresult = RegOpenKeyExA(HKEY_CURRENT_USER, (char *)path.str, 0, KEY_SET_VALUE, &handle);
	b32 result = hresult == ERROR_SUCCESS;
	if(result)
	{
		hresult =  RegDeleteValueA(handle, (char *)key.str);
		result = hresult == ERROR_SUCCESS;
		RegCloseKey(handle);
	}
	return result;
}

internal b32
w32_hkcu_write(String8 path, String8 key, String8 value)
{
	HKEY handle;
    LONG hresult = RegCreateKeyExA(HKEY_CURRENT_USER, (char *)path.str, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &handle, 0);
	b32 result = hresult == ERROR_SUCCESS;
	if(result)
	{
		hresult = RegSetValueExA(handle, (char *)key.str, 0, REG_BINARY, (LPBYTE)value.str, value.size);
		result = hresult == ERROR_SUCCESS;
		RegCloseKey(handle);
	}
	return result;
}

internal String8
w32_hkcu_read(Arena *arena, String8 path, String8 name)
{
	HKEY handle;
	LONG hresult = RegOpenKeyExA(HKEY_CURRENT_USER, (char *)path.str, 0, KEY_READ, &handle);
	String8 result = {0};
	if(hresult == ERROR_SUCCESS)
	{
		DWORD type = {0};
		DWORD size = {0};
		hresult = RegQueryValueExA(handle, (char *)name.str, 0, &type, 0, &size);
		if(hresult == ERROR_SUCCESS && size > 0)
		{
			u8 *buffer = push_array(arena, u8, size);
			hresult = RegQueryValueExA(handle, (char *)name.str, 0, &type, (LPBYTE)buffer, &size);
			result = str8_cstring((char *)buffer);
			RegCloseKey(handle);
		}
	}
	return result;
}

internal String8
w32_hklm_read(Arena *arena, String8 path, String8 name)
{
	HKEY handle;
	LONG hresult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, (char *)path.str, 0, KEY_READ, &handle);
	String8 result = {0};
	if(hresult == ERROR_SUCCESS)
	{
		DWORD type = {0};
		DWORD size = {0};
		hresult = RegQueryValueExA(handle, (char *)name.str, 0, &type, 0, &size);
		if(hresult == ERROR_SUCCESS && size > 0)
		{
			u8 *buffer = push_array(arena, u8, size);
			hresult = RegQueryValueExA(handle, (char *)name.str, 0, &type, (LPBYTE)buffer, &size);
			result = str8_cstring((char *)buffer);
			RegCloseKey(handle);
		}
	}
	return result;
}

internal String8List
w32_hklm_list_keys(Arena *arena, String8 path)
{
	HKEY handle;
	LONG hresult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, (char *)path.str, 0, KEY_QUERY_VALUE, &handle); 
	String8List result = {0};
	if(hresult == ERROR_SUCCESS)
	{
		//- rsb: get max sub key size and sub key count
		DWORD max_sub_value_size = {0};
		DWORD sub_value_count = {0};
		hresult = RegQueryInfoKeyA(handle, 0, 0, 0, 0, 0, 0, &sub_value_count, &max_sub_value_size, 0, 0, 0);
		max_sub_value_size += 1;
		
		//- rsb: get each key name
		for(u32 it = 0; it < sub_value_count && hresult == ERROR_SUCCESS; it += 1)
		{
			u8 *buffer = push_array(arena, u8, max_sub_value_size);
			DWORD size = max_sub_value_size;
			hresult = RegEnumValueA(handle, it, (LPSTR)buffer, &size, 0, 0, 0, 0);
			str8_list_push(arena, &result, str8_cstring((char *)buffer));
		}
		RegCloseKey(handle);
	}
	return result;
}

internal String8
os_get_hwid(Arena *arena)
{
	HW_PROFILE_INFOA hwinfo;
	GetCurrentHwProfileA(&hwinfo);
	String8 id = str8((u8 *)hwinfo.szHwProfileGuid, ArrayCount(hwinfo.szHwProfileGuid));
	id = str8_skip(id, 1);
	id = str8_chop(id, 2);
	String8 result = push_str8_copy(arena, id);
	return result;
}

//
//~ rsb: @os_hooks Memory Allocation (Implemented per OS)
//

internal void *
os_reserve(u64 size)
{
	void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	return result;
}

internal b32
os_commit(void *ptr, u64 size)
{
	b32 result = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
	return result;
}

internal void
os_decommit(void *ptr, u64 size)
{
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

internal void
os_release(void *ptr, u64 size)
{
	//- rsb: Size not used for win32 but for other OSes
	VirtualFree(ptr, 0, MEM_RELEASE);
	(void)size;
}

//
//~ rsb: @os_hooks System info
//

internal OS_SystemInfo *
os_get_system_info()
{
	return &os_w32_state.system_info;
}

//
//~ rsb: @os_hooks Entry point
//

internal void
w32_entry_point(void)
{
	{
		os_w32_state.microsecond_resolution  = 1;
		LARGE_INTEGER large_int_resolution;
		if(QueryPerformanceFrequency(&large_int_resolution))
		{
			os_w32_state.microsecond_resolution = large_int_resolution.QuadPart;
		}
		os_w32_state.granular_sleep_enabled = (timeBeginPeriod(1) == TIMERR_NOERROR);
	}
	
	//- rsb: Setup system info
	{
		SYSTEM_INFO sysinfo = {0};
		GetSystemInfo(&sysinfo);
		OS_SystemInfo *info = &os_w32_state.system_info;
		info->page_size               = sysinfo.dwPageSize;
		info->logical_processor_count = (u64) sysinfo.dwNumberOfProcessors;
		info->allocation_granularity  = sysinfo.dwAllocationGranularity;
	}
	
	//- rjf: set up entity storage
	InitializeCriticalSection(&os_w32_state.entity_mutex);
	os_w32_state.entity_arena = arena_alloc_default();
	
	//- rsb: Setup thread context
	local_persist TCTX tctx;
	tctx_init_and_equip(&tctx);
}

////////////////////////////////
//~ rsb: Joystick Functions

internal r32 
os_joystick_correction(i16 value, i16 deadzone)
{
    r32 result = 0;
    if(value > deadzone)
	{
        result = ((r32)value - (r32)deadzone) / (32767.0f - (r32)deadzone);
    }
	else if(value < -deadzone)
	{
        result = ((r32)value + (r32)deadzone) / (32768.0f - (r32)deadzone);
    }
    return result;
}

internal b32 
os_joystick_button(DWORD xinput_button_state, OS_Button *prev, DWORD button_bit, OS_Button *curr)
{
    curr->pressed = ((xinput_button_state & button_bit) == button_bit);
    curr->half_transition_count = (prev->pressed != curr->pressed) ? 1 : 0;
    return curr->pressed;
}

internal OS_Joystick *
os_joystick_state(void)
{
	local_persist OS_Joystick joystick[2] = {0};
	local_persist OS_Joystick *curr = &joystick[0];
	local_persist OS_Joystick *prev = &joystick[1];
	
	//- rsb: update state
    XINPUT_STATE state;
	MemoryZeroStruct(curr);
	curr->error = XInputGetState(0, &state);
    curr->connected = !(curr->error == ERROR_DEVICE_NOT_CONNECTED);
    if(curr->connected)
	{
        XINPUT_GAMEPAD *pad = &state.Gamepad;
        curr->packet_no = state.dwPacketNumber;
        curr->lx = os_joystick_correction(pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        curr->ly = os_joystick_correction(pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        curr->rx = os_joystick_correction(pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        curr->ry = os_joystick_correction(pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->up, XINPUT_GAMEPAD_DPAD_UP, &curr->up);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->down, XINPUT_GAMEPAD_DPAD_DOWN, &curr->down);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->left, XINPUT_GAMEPAD_DPAD_LEFT, &curr->left);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->right, XINPUT_GAMEPAD_DPAD_RIGHT, &curr->right);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->start, XINPUT_GAMEPAD_START, &curr->start);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->back, XINPUT_GAMEPAD_BACK, &curr->back);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->lb, XINPUT_GAMEPAD_LEFT_SHOULDER, &curr->lb);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->rb, XINPUT_GAMEPAD_RIGHT_SHOULDER, &curr->rb);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->a, XINPUT_GAMEPAD_A, &curr->a);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->b, XINPUT_GAMEPAD_B, &curr->b);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->x, XINPUT_GAMEPAD_X, &curr->x);
        curr->press_count += os_joystick_button(pad->wButtons, &prev->y, XINPUT_GAMEPAD_Y, &curr->y);
        curr->ltrigger = pad->bLeftTrigger;
        curr->rtrigger = pad->bRightTrigger;
    } 
	
	//- rsb: send current state
	OS_Joystick *result = curr;
	Swap(OS_Joystick *, curr, prev);
	return result;
}

////////////////////////////////
//~ rjf: Entity Functions

internal OS_W32_Entity *
os_w32_entity_alloc(OS_W32_EntityKind kind)
{
	OS_W32_Entity *result = 0;
	EnterCriticalSection(&os_w32_state.entity_mutex);
	{
		result = os_w32_state.entity_free;
		if(result)
		{
			SLLStackPop(os_w32_state.entity_free);
		}
		else
		{
			result = push_array_no_zero(os_w32_state.entity_arena, OS_W32_Entity, 1);
		}
		MemoryZeroStruct(result);
	}
	LeaveCriticalSection(&os_w32_state.entity_mutex);
	result->kind = kind;
	return result;
}

internal void
os_w32_entity_release(OS_W32_Entity *entity)
{
	entity->kind = OS_W32_EntityKind_Null;
	EnterCriticalSection(&os_w32_state.entity_mutex);
	SLLStackPush(os_w32_state.entity_free, entity);
	LeaveCriticalSection(&os_w32_state.entity_mutex);
}

////////////////////////////////
//~ rjf: Time Conversion Helpers

internal U32
os_w32_sleep_ms_from_endt_us(U64 endt_us)
{
	U32 sleep_ms = 0;
	if(endt_us == max_U64)
	{
		sleep_ms = INFINITE;
	}
	else
	{
		U64 begint = os_now_microseconds();
		if(begint < endt_us)
		{
			U64 sleep_us = endt_us - begint;
			sleep_ms = (U32)((sleep_us + 999)/1000);
		}
	}
	return sleep_ms;
}

internal U32
os_w32_unix_time_from_file_time(FILETIME file_time)
{
	U64 win32_time = ((U64)file_time.dwHighDateTime << 32) | file_time.dwLowDateTime;
	U64 unix_time64 = ((win32_time - 0x19DB1DED53E8000ULL) / 10000000);
	
	Assert(unix_time64 <= max_U32);
	U32 unix_time32 = (U32)unix_time64;
	
	return unix_time32;
}

////////////////////////////////
//~ rjf: @os_hooks Time (Implemented Per-OS)

internal U64
os_now_microseconds(void)
{
	U64 result = 0;
	LARGE_INTEGER large_int_counter;
	if(QueryPerformanceCounter(&large_int_counter))
	{
		result = (large_int_counter.QuadPart*Million(1))/os_w32_state.microsecond_resolution;
	}
	return result;
}

internal void
os_sleep_milliseconds(U32 msec)
{
	Sleep(msec);
}

internal u64
os_now_unix(void)
{
	u64 result = time(0);
	return result;
}

internal r64
os_time_diff(u64 end, u64 start)
{
	r64 result = difftime(end, start);
	return result;
}

internal DateTime 
os_local_time_from_timestamp(u64 timestamp)
{
    DateTime result = {0};
	tm *ctime = localtime((time_t *)&timestamp);
    result.sec = ctime->tm_sec;
    result.min = ctime->tm_min;
    result.hour = ctime->tm_hour;
	result.day = ctime->tm_mday;
    result.wday = ctime->tm_wday;
    result.mon = ctime->tm_mon;
    result.year = ctime->tm_year + 1900;
	return result;
}

internal DateTime 
os_local_time()
{
	u64 local_time = os_now_unix();
	DateTime result = os_local_time_from_timestamp(local_time);
	return result;
}

internal void
os_wait(U64 end_time_us)
{
	U64 begin_time_us = os_now_microseconds();
	if(end_time_us > begin_time_us)
	{
		U64 time_to_wait_us = end_time_us - begin_time_us;
		if(os_w32_state.granular_sleep_enabled)
		{
			os_sleep_milliseconds(time_to_wait_us/1000);
		}
		else
		{
			os_sleep_milliseconds(time_to_wait_us/15000);
		}
		for(;os_now_microseconds() < end_time_us;);
	}
}

////////////////////////////////
//~ rjf: Thread Entry Point

internal DWORD
os_w32_thread_entry_point(void *ptr)
{
	OS_W32_Entity *entity = (OS_W32_Entity *)ptr;
	OS_ThreadFunctionType *func = entity->thread.func;
	void *thread_ptr = entity->thread.ptr;
	TCTX tctx_;
	tctx_init_and_equip(&tctx_);
	func(thread_ptr);
	tctx_release();
	return 0;
}

////////////////////////////////
//~ rjf: @os_hooks Threads (Implemented Per-OS)

internal OS_Handle
os_thread_launch(OS_ThreadFunctionType *func, void *ptr, void *params)
{
	OS_W32_Entity *entity = os_w32_entity_alloc(OS_W32_EntityKind_Thread);
	entity->thread.func = func;
	entity->thread.ptr = ptr;
	entity->thread.handle = CreateThread(0, 0, os_w32_thread_entry_point, entity, 0, &entity->thread.tid);
	OS_Handle result = {IntFromPtr(entity)};
	return result;
}

internal B32
os_thread_join(OS_Handle handle, U64 endt_us)
{
	DWORD sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
	OS_W32_Entity *entity = (OS_W32_Entity *)PtrFromInt(handle.u64[0]);
	DWORD wait_result = WAIT_OBJECT_0;
	if(entity != 0)
	{
		wait_result = WaitForSingleObject(entity->thread.handle, sleep_ms);
		CloseHandle(entity->thread.handle);
		os_w32_entity_release(entity);
	}
	return (wait_result == WAIT_OBJECT_0);
}

internal void
os_thread_detach(OS_Handle thread)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(thread.u64[0]);
	if(entity != 0)
	{
		CloseHandle(entity->thread.handle);
		os_w32_entity_release(entity);
	}
}

////////////////////////////////
//~ rjf: @os_hooks Synchronization Primitives (Implemented Per-OS)

//- rjf: mutexes
internal OS_Handle
os_mutex_alloc(void)
{
	OS_W32_Entity *entity = os_w32_entity_alloc(OS_W32_EntityKind_Mutex);
	InitializeCriticalSection(&entity->mutex);
	OS_Handle result = {IntFromPtr(entity)};
	return result;
}

internal void
os_mutex_release(OS_Handle mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(mutex.u64[0]);
	os_w32_entity_release(entity);
}

internal void
os_mutex_take(OS_Handle mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(mutex.u64[0]);
	EnterCriticalSection(&entity->mutex);
}

internal void
os_mutex_drop(OS_Handle mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(mutex.u64[0]);
	LeaveCriticalSection(&entity->mutex);
}

//- rjf: reader/writer mutexes

internal OS_Handle
os_rw_mutex_alloc(void)
{
	OS_W32_Entity *entity = os_w32_entity_alloc(OS_W32_EntityKind_RWMutex);
	InitializeSRWLock(&entity->rw_mutex);
	OS_Handle result = {IntFromPtr(entity)};
	return result;
}

internal void
os_rw_mutex_release(OS_Handle rw_mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
	os_w32_entity_release(entity);
}

internal void
os_rw_mutex_take_r(OS_Handle rw_mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
	AcquireSRWLockShared(&entity->rw_mutex);
}

internal void
os_rw_mutex_drop_r(OS_Handle rw_mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
	ReleaseSRWLockShared(&entity->rw_mutex);
}

internal void
os_rw_mutex_take_w(OS_Handle rw_mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
	AcquireSRWLockExclusive(&entity->rw_mutex);
}

internal void
os_rw_mutex_drop_w(OS_Handle rw_mutex)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(rw_mutex.u64[0]);
	ReleaseSRWLockExclusive(&entity->rw_mutex);
}

//- rjf: condition variables

internal OS_Handle
os_condition_variable_alloc(void)
{
	OS_W32_Entity *entity = os_w32_entity_alloc(OS_W32_EntityKind_ConditionVariable);
	InitializeConditionVariable(&entity->cv);
	OS_Handle result = {IntFromPtr(entity)};
	return result;
}

internal void
os_condition_variable_release(OS_Handle cv)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(cv.u64[0]);
	os_w32_entity_release(entity);
}

internal B32
os_condition_variable_wait(OS_Handle cv, OS_Handle mutex, U64 endt_us)
{
	U32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
	BOOL result = 0;
	if(sleep_ms > 0)
	{
		OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(cv.u64[0]);
		OS_W32_Entity *mutex_entity = (OS_W32_Entity*)PtrFromInt(mutex.u64[0]);
		result = SleepConditionVariableCS(&entity->cv, &mutex_entity->mutex, sleep_ms);
	}
	return result;
}

internal B32
os_condition_variable_wait_rw_r(OS_Handle cv, OS_Handle mutex_rw, U64 endt_us)
{
	U32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
	BOOL result = 0;
	if(sleep_ms > 0)
	{
		OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(cv.u64[0]);
		OS_W32_Entity *mutex_entity = (OS_W32_Entity*)PtrFromInt(mutex_rw.u64[0]);
		result = SleepConditionVariableSRW(&entity->cv, &mutex_entity->rw_mutex, sleep_ms,
										   CONDITION_VARIABLE_LOCKMODE_SHARED);
	}
	return result;
}

internal B32
os_condition_variable_wait_rw_w(OS_Handle cv, OS_Handle mutex_rw, U64 endt_us)
{
	U32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
	BOOL result = 0;
	if(sleep_ms > 0)
	{
		OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(cv.u64[0]);
		OS_W32_Entity *mutex_entity = (OS_W32_Entity*)PtrFromInt(mutex_rw.u64[0]);
		result = SleepConditionVariableSRW(&entity->cv, &mutex_entity->rw_mutex, sleep_ms, 0);
	}
	return result;
}

internal void
os_condition_variable_signal(OS_Handle cv)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(cv.u64[0]);
	WakeConditionVariable(&entity->cv);
}

internal void
os_condition_variable_broadcast(OS_Handle cv)
{
	OS_W32_Entity *entity = (OS_W32_Entity*)PtrFromInt(cv.u64[0]);
	WakeAllConditionVariable(&entity->cv);
}

//- rjf: cross-process semaphores

internal OS_Handle
os_semaphore_alloc(U32 initial_count, U32 max_count, String8 name)
{
	Temp scratch = scratch_begin(0);
	String16 name16 = str16_from_8(scratch.arena, name);
	HANDLE handle = CreateSemaphoreW(0, initial_count, max_count, (WCHAR *)name16.str);
	OS_Handle result = {(U64)handle};
	scratch_end(scratch);
	return result;
}

internal void
os_semaphore_release(OS_Handle semaphore)
{
	HANDLE handle = (HANDLE)semaphore.u64[0];
	CloseHandle(handle);
}

internal OS_Handle
os_semaphore_open(String8 name)
{
	Temp scratch = scratch_begin(0);
	String16 name16 = str16_from_8(scratch.arena, name);
	HANDLE handle = OpenSemaphoreW(SEMAPHORE_ALL_ACCESS , 0, (WCHAR *)name16.str);
	OS_Handle result = {(U64)handle};
	scratch_end(scratch);
	return result;
}

internal void
os_semaphore_close(OS_Handle semaphore)
{
	HANDLE handle = (HANDLE)semaphore.u64[0];
	CloseHandle(handle);
}

internal B32
os_semaphore_take(OS_Handle semaphore, U64 endt_us)
{
	U32 sleep_ms = os_w32_sleep_ms_from_endt_us(endt_us);
	HANDLE handle = (HANDLE)semaphore.u64[0];
	DWORD wait_result = WaitForSingleObject(handle, sleep_ms);
	B32 result = (wait_result == WAIT_OBJECT_0);
	return result;
}

internal void
os_semaphore_drop(OS_Handle semaphore)
{
	HANDLE handle = (HANDLE)semaphore.u64[0];
	ReleaseSemaphore(handle, 1, 0);
}

//
//~ rsb: @os_hooks File and folder functions
//

internal DenseTime
dense_time_from_date_time(DateTime date_time)
{
	DenseTime result = {0};
	result += date_time.year;
	result *= 12;
	result += date_time.mon;
	result *= 31;
	result += date_time.day;
	result *= 24;
	result += date_time.hour;
	result *= 60;
	result += date_time.min;
	result *= 61;
	result += date_time.sec;
	result *= 1000;
	result += date_time.msec;
	return result;
}

internal DateTime
date_time_from_dense_time(DenseTime time)
{
	DateTime result = {0};
	result.msec = time % 1000;
	time /= 1000;
	result.sec  = time % 61;
	time /= 61;
	result.min  = time % 60;
	time /= 60;
	result.hour = time % 24;
	time /= 24;
	result.day  = time % 31;
	time /= 31;
	result.mon  = time % 12;
	time /= 12;
	Assert(time <= max_U32);
	result.year = (u32)time;
	return result ;
}

internal void
os_w32_system_time_from_date_time(SYSTEMTIME *out, DateTime *in)
{
	out->wYear         = (WORD)(in->year);
	out->wMonth        = in->mon + 1;
	out->wDay          = in->day;
	out->wHour         = in->hour;
	out->wMinute       = in->min;
	out->wSecond       = in->sec;
	out->wMilliseconds = in->msec;
}

internal void
os_w32_date_time_from_system_time(DateTime *out, SYSTEMTIME *in)
{
	out->year  = in->wYear;
	out->mon   = in->wMonth - 1;
	out->wday  = in->wDayOfWeek;
	out->day   = in->wDay;
	out->hour  = in->wHour;
	out->min   = in->wMinute;
	out->sec   = in->wSecond;
	out->msec  = in->wMilliseconds;
}

internal DenseTime
os_w32_dense_time(void)
{
	SYSTEMTIME system_time;
	GetSystemTime(&system_time);
	DateTime date_time = {0};
	os_w32_date_time_from_system_time(&date_time, &system_time);
	DenseTime result = dense_time_from_date_time(date_time);
	return result;
}

internal void 
os_w32_dense_time_from_file_time(DenseTime *out, FILETIME *in)
{
	SYSTEMTIME system_time;
	FileTimeToSystemTime(in, &system_time);
	DateTime date_time = {0};
	os_w32_date_time_from_system_time(&date_time, &system_time);
	*out = dense_time_from_date_time(date_time);
}

internal FilePropertyFlags
os_w32_file_property_flags_from_dwFileAttributes(DWORD attributes)
{
	FilePropertyFlags flags = {0};
	if(attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		flags |= FilePropertyFlag_IsFolder;
	}
	return flags;
}

internal OS_Handle
os_file_open(OS_AccessFlags flags, String8 path)
{
	OS_Handle result = {0};
	Temp scratch = scratch_begin(0);
	String16 path16 = str16_from_8(scratch.arena, path);
	DWORD access_flags = 0;
	DWORD share_mode = 0;
	DWORD creation_disposition = OPEN_EXISTING;
	SECURITY_ATTRIBUTES security_attributes = {sizeof(security_attributes), 0, 0};
	if(flags & OS_AccessFlag_Read)        {access_flags |= GENERIC_READ;}
	if(flags & OS_AccessFlag_Write)       {access_flags |= GENERIC_WRITE;}
	if(flags & OS_AccessFlag_Execute)     {access_flags |= GENERIC_EXECUTE;}
	if(flags & OS_AccessFlag_ShareRead)   {share_mode |= FILE_SHARE_READ;}
	if(flags & OS_AccessFlag_ShareWrite)  {share_mode |= FILE_SHARE_WRITE|FILE_SHARE_DELETE;}
	if(flags & OS_AccessFlag_Write)       {creation_disposition = CREATE_ALWAYS;}
	if(flags & OS_AccessFlag_Append)      {creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA; }
	if(flags & OS_AccessFlag_Inherited)
	{
		security_attributes.bInheritHandle = 1;
	}
	HANDLE file = CreateFileW((WCHAR *)path16.str, access_flags, share_mode, &security_attributes, creation_disposition, FILE_ATTRIBUTE_NORMAL, 0);
	if(file != INVALID_HANDLE_VALUE)
	{
		result.u64[0] = (U64)file;
	}
	scratch_end(scratch);
	return result;
}

internal void
os_file_close(OS_Handle file)
{
	if(os_handle_match(file, os_handle_zero())) { return; }
	HANDLE handle = (HANDLE)file.u64[0];
	BOOL result = CloseHandle(handle);
	(void)result;
}

internal U64
os_file_read(OS_Handle file, Rng1U64 rng, void *out_data)
{
	if(os_handle_match(file, os_handle_zero())) { return 0; }
	HANDLE handle = (HANDLE)file.u64[0];
	
	// rjf: clamp range by file size
	U64 size = 0;
	GetFileSizeEx(handle, (LARGE_INTEGER *)&size);
	Rng1U64 rng_clamped  = r1u64(ClampTop(rng.min, size), ClampTop(rng.max, size));
	U64 total_read_size = 0;
	
	// rjf: read loop
	{
		U64 to_read = dim_1u64(rng_clamped);
		for(U64 off = rng.min; total_read_size < to_read;)
		{
			U64 amt64 = to_read - total_read_size;
			U32 amt32 = u32_from_u64_saturate(amt64);
			DWORD read_size = 0;
			OVERLAPPED overlapped = {0};
			overlapped.Offset     = (off&0x00000000ffffffffull);
			overlapped.OffsetHigh = (off&0xffffffff00000000ull) >> 32;
			ReadFile(handle, (U8 *)out_data + total_read_size, amt32, &read_size, &overlapped);
			off += read_size;
			total_read_size += read_size;
			if(read_size != amt32)
			{
				break;
			}
		}
	}
	
	return total_read_size;
}

internal U64
os_file_write(OS_Handle file, Rng1U64 rng, void *data)
{
	if(os_handle_match(file, os_handle_zero())) { return 0; }
	HANDLE win_handle = (HANDLE)file.u64[0];
	U64 src_off = 0;
	U64 dst_off = rng.min;
	U64 total_write_size = dim_1u64(rng);
	for(;;)
	{
		void *bytes_src = (U8 *)data + src_off;
		U64 bytes_left = total_write_size - src_off;
		DWORD write_size = Min(MB(1), bytes_left);
		DWORD bytes_written = 0;
		OVERLAPPED overlapped = {0};
		overlapped.Offset = (dst_off&0x00000000ffffffffull);
		overlapped.OffsetHigh = (dst_off&0xffffffff00000000ull) >> 32;
		BOOL success = WriteFile(win_handle, bytes_src, write_size, &bytes_written, &overlapped);
		if(success == 0)
		{
			break;
		}
		src_off += bytes_written;
		dst_off += bytes_written;
		if(bytes_left == 0)
		{
			break;
		}
	}
	return src_off;
}

internal B32
os_file_set_time(OS_Handle file, DateTime time)
{
	if(os_handle_match(file, os_handle_zero())) { return 0; }
	B32 result = 0;
	HANDLE handle = (HANDLE)file.u64[0];
	SYSTEMTIME system_time = {0};
	os_w32_system_time_from_date_time(&system_time, &time);
	FILETIME file_time = {0};
	result = (SystemTimeToFileTime(&system_time, &file_time) &&
			  SetFileTime(handle, &file_time, &file_time, &file_time));
	return result;
}

internal FileProperties
os_properties_from_file(OS_Handle file)
{
	if(os_handle_match(file, os_handle_zero())) { FileProperties r = {0}; return r; }
	FileProperties props = {0};
	HANDLE handle = (HANDLE)file.u64[0];
	BY_HANDLE_FILE_INFORMATION info;
	BOOL info_good = GetFileInformationByHandle(handle, &info);
	if(info_good)
	{
		U32 size_lo = info.nFileSizeLow;
		U32 size_hi = info.nFileSizeHigh;
		props.size     = (U64)size_lo | (((U64)size_hi)<<32);
		os_w32_dense_time_from_file_time(&props.modified, &info.ftLastWriteTime);
		os_w32_dense_time_from_file_time(&props.created, &info.ftCreationTime);
		props.flags = os_w32_file_property_flags_from_dwFileAttributes(info.dwFileAttributes);
	}
	return props;
}

internal OS_FileID
os_id_from_file(OS_Handle file)
{
	if(os_handle_match(file, os_handle_zero())) { OS_FileID r = {0}; return r; }
	OS_FileID result = {0};
	HANDLE handle = (HANDLE)file.u64[0];
	BY_HANDLE_FILE_INFORMATION info;
	BOOL is_ok = GetFileInformationByHandle(handle, &info);
	if(is_ok)
	{
		result.v[0] = info.dwVolumeSerialNumber;
		result.v[1] = info.nFileIndexLow;
		result.v[2] = info.nFileIndexHigh;
	}
	return result;
}

internal B32
os_file_reserve_size(OS_Handle file, U64 size)
{
	HANDLE handle = (HANDLE)file.u64[0];
	
	FILE_ALLOCATION_INFO alloc_info    = {0};
	alloc_info.AllocationSize.LowPart  = size & max_U32;
	alloc_info.AllocationSize.HighPart = (size >> 32) & max_U32;
	
	BOOL is_reserved = SetFileInformationByHandle(handle, FileAllocationInfo, &alloc_info, sizeof(alloc_info));
	return is_reserved;
}

internal B32
os_delete_file_at_path(String8 path)
{
	Temp scratch = scratch_begin(0);
	String16 path16 = str16_from_8(scratch.arena, path);
	B32 result = DeleteFileW((WCHAR*)path16.str);
	scratch_end(scratch);
	return result;
}

internal B32
os_copy_file_path(String8 dst, String8 src)
{
	Temp scratch = scratch_begin(0);
	String16 dst16 = str16_from_8(scratch.arena, dst);
	String16 src16 = str16_from_8(scratch.arena, src);
	B32 result = CopyFileW((WCHAR*)src16.str, (WCHAR*)dst16.str, 0);
	scratch_end(scratch);
	return result;
}

internal B32
os_move_file_path(String8 dst, String8 src)
{
	Temp scratch = scratch_begin(0);
	String16 dst16 = str16_from_8(scratch.arena, dst);
	String16 src16 = str16_from_8(scratch.arena, src);
	B32 result = MoveFileW((WCHAR*)src16.str, (WCHAR*)dst16.str);
	scratch_end(scratch);
	return result;
}

internal String8
os_full_path_from_path(Arena *arena, String8 path)
{
	Temp scratch = scratch_begin(arena);
	DWORD     buffer_size = Max(MAX_PATH, path.size * 2) + 1;
	String16  path16      = str16_from_8(scratch.arena, path);
	WCHAR    *buffer      = push_array_no_zero(scratch.arena, WCHAR, buffer_size);
	DWORD     path16_size = GetFullPathNameW((WCHAR*)path16.str, buffer_size, buffer, NULL);
	if(path16_size > buffer_size)
	{
		arena_pop(scratch.arena, buffer_size);
		buffer_size = path16_size + 1;
		buffer      = push_array_no_zero(scratch.arena, WCHAR, buffer_size);
		path16_size = GetFullPathNameW((WCHAR*)path16.str, buffer_size, buffer, NULL);
	}
	String8 full_path = str8_from_16(arena, str16((U16*)buffer, path16_size));
	scratch_end(scratch);
	return full_path;
}

internal B32
os_file_path_exists(String8 path)
{
	Temp scratch = scratch_begin(0);
	String16 path16 = str16_from_8(scratch.arena, path);
	DWORD attributes = GetFileAttributesW((WCHAR *)path16.str);
	B32 exists = (attributes != INVALID_FILE_ATTRIBUTES) && !!(~attributes & FILE_ATTRIBUTE_DIRECTORY);
	scratch_end(scratch);
	return exists;
}

internal B32
os_folder_path_exists(String8 path)
{
	Temp scratch = scratch_begin(0);
	String16 path16     = str16_from_8(scratch.arena, path);
	DWORD    attributes = GetFileAttributesW((WCHAR *)path16.str);
	B32      exists     = (attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY);
	scratch_end(scratch);
	return exists;
}

internal FileProperties
os_properties_from_file_path(String8 path)
{
	WIN32_FIND_DATAW find_data = {0};
	Temp scratch = scratch_begin(0);
	String16 path16 = str16_from_8(scratch.arena, path);
	HANDLE handle = FindFirstFileW((WCHAR *)path16.str, &find_data);
	FileProperties props = {0};
	if(handle != INVALID_HANDLE_VALUE)
	{
		props.size = Compose64Bit(find_data.nFileSizeHigh, find_data.nFileSizeLow);
		os_w32_dense_time_from_file_time(&props.created, &find_data.ftCreationTime);
		os_w32_dense_time_from_file_time(&props.modified, &find_data.ftLastWriteTime);
		props.flags = os_w32_file_property_flags_from_dwFileAttributes(find_data.dwFileAttributes);
	}
	else
	{
		Temp scratch2 = scratch_begin(0);
		WCHAR buffer[512] = {0};
		DWORD length = GetLogicalDriveStringsW(sizeof(buffer), buffer);
		U64 last_slash_pos = 0;
		for(;last_slash_pos < path.size; last_slash_pos = str8_find_needle(path, last_slash_pos+1, str8_lit("/"), StringMatchFlag_SlashInsensitive));
		String8 path_trimmed = str8_prefix(path, last_slash_pos);
		for(U64 off = 0; off < (U64)length;)
		{
			String16 next_drive_string_16 = str16_cstring((U16 *)buffer+off);
			off += next_drive_string_16.size+1;
			String8 next_drive_string = str8_from_16(scratch2.arena, next_drive_string_16);
			next_drive_string = str8_chop_last_slash(next_drive_string);
			if(str8_match(path_trimmed, next_drive_string, StringMatchFlag_CaseInsensitive))
			{
				props.flags |= FilePropertyFlag_IsFolder;
				break;
			}
		}
		scratch_end(scratch2);
	}
	FindClose(handle);
	scratch_end(scratch);
	return props;
}

internal b32
os_make_directory(String8 path)
{
	b32 result = 1;
	Temp temp = scratch_begin(0);
	String8 ascii_path = push_str8_copy(temp.arena, path);
	if(!CreateDirectoryA((char *)ascii_path.str, 0))
	{
		DWORD error = GetLastError();
		if(error == ERROR_PATH_NOT_FOUND)
		{
			result = 0;
		}
	}
	scratch_end(temp);
	return result;
}

internal OS_FileIter *
os_file_iter_begin(Arena *arena, String8 path, OS_FileIterFlags flags)
{
	Temp temp = scratch_begin(arena);
	OS_FileIter *iter = push_array(arena, OS_FileIter, 1);
	String8 path_with_wildcard = push_str8_cat(temp.arena, path, str8_lit("/*"));
	if(path_with_wildcard.size != 0)
	{
		iter->flags = flags;
		OS_W32_FileIter *w32_iter = (OS_W32_FileIter *)iter->memory;
		w32_iter->handle = FindFirstFileA((char *)path_with_wildcard.str, &w32_iter->find_data);
	}
	scratch_end(temp);
	return iter;
}

internal b32
os_file_iter_next(Arena *arena, OS_FileIter *iter, OS_FileInfo *info)
{
	OS_W32_FileIter *w32_iter = (OS_W32_FileIter *)iter->memory;
	b32 result = 0;
	if(!(iter->flags & OS_FileIterFlag_Done) && w32_iter->handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			//- rsb: Check if file is usable
			b32 usable_file = 1;
			char *file_name = w32_iter->find_data.cFileName;
			DWORD attributes = w32_iter->find_data.dwFileAttributes;
			if(file_name[0] == '.')
			{
				if(iter->flags & OS_FileIterFlag_SkipHiddenFiles)
				{
					usable_file = 0;
				}
				else if(file_name[1] == 0)
				{
					usable_file = 0;
				}
				else if(file_name[1] == '.' && file_name[2] == 0)
				{
					usable_file = 0;
				}
			}
			if(attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(iter->flags & OS_FileIterFlag_SkipFolders)
				{
					usable_file = 0;
				}
			}
			else
			{
				if(iter->flags & OS_FileIterFlag_SkipFiles)
				{
					usable_file = 0;
				}
			}
			
			//- rsb: Omit if unusable
			if(usable_file)
			{
				result = 1;
				info->name = push_str8f(arena, "%s", file_name);
				info->props.size = (((u64)w32_iter->find_data.nFileSizeHigh << 32) | (u64)w32_iter->find_data.nFileSizeLow);
				os_w32_dense_time_from_file_time(&info->props.modified, &w32_iter->find_data.ftLastWriteTime);
				os_w32_dense_time_from_file_time(&info->props.created, &w32_iter->find_data.ftCreationTime);
				info->props.flags = os_w32_file_property_flags_from_dwFileAttributes(attributes);
				if(!FindNextFileA(w32_iter->handle, &w32_iter->find_data))
				{
					iter->flags |= OS_FileIterFlag_Done;
				}
				break;
			}
		}
		while(FindNextFileA(w32_iter->handle, &w32_iter->find_data));
	}
	if(!result)
	{
		iter->flags |= OS_FileIterFlag_Done;
	}
	return result;
}

internal void
os_file_iter_end(OS_FileIter *it)
{
	OS_W32_FileIter *w32_iter = (OS_W32_FileIter *)it->memory;
	HANDLE zero_handle = {0};
	if(w32_iter->handle != zero_handle)
	{
		FindClose(w32_iter->handle);
	}
}

internal String8
os_file_open_dialog(Arena *arena,
                    String8 filter,
                    String8 title)
{
    char file_buf[MAX_PATH] = {0};
	
    OPENFILENAMEA ofn = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.lpstrFile    = file_buf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrFilter  = (const char *)filter.str;
    ofn.lpstrTitle   = (const char *)title.str;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
    if (GetOpenFileNameA(&ofn))
    {
        return push_str8_copy(arena, str8_cstring(file_buf));
    }
	
    return {};
}


//
//~ rsb: Misc
//

internal void
os_open_in_browser(String8 url)
{
	Temp scratch = scratch_begin(0);
	String8 string = push_str8_copy(scratch.arena, url);
	ShellExecuteA(0, "open", (char *)string.str, 0, 0, SW_SHOWNORMAL);
	scratch_end(scratch);
}

internal String8List
os_get_mac_addresses(Arena *arena)
{
	Temp temp = scratch_begin(arena);
	String8List result = {0};
	ULONG size = {0};
	GetAdaptersInfo(0, &size);
	IP_ADAPTER_INFO *adapter_info = (IP_ADAPTER_INFO *)push_array(temp.arena, u8, size);
	if(GetAdaptersInfo(adapter_info, &size) == NO_ERROR)
	{
		for(; adapter_info != 0; adapter_info = adapter_info->Next)
		{
			String8 str = push_str8_copy(arena, str8(adapter_info->Address, adapter_info->AddressLength));
			str8_list_push(arena, &result, str);
		}
	}
	scratch_end(temp);
	return result;
}

internal String8
os_get_current_path(Arena *arena)
{
	Temp scratch = scratch_begin(arena);
	DWORD length = GetCurrentDirectoryW(0, 0);
	U16 *memory = push_array_no_zero(scratch.arena, U16, length + 1);
	length = GetCurrentDirectoryW(length + 1, (WCHAR*)memory);
	String8 name = str8_from_16(arena, str16(memory, length));
	scratch_end(scratch);
	return name;
}

#if 0
internal String8
os_get_username(Arena *arena)
{
	DWORD size = {0};
	EXTENDED_NAME_FORMAT format = NameDisplay;
	GetUserNameExA(format, 0, &size);
	char *str = push_array(arena, char, size);
	GetUserNameExA(format, str, &size);
	String8 result = str8((u8 *)str, (u64)size);
	return result;
}
#endif

internal String8
os_get_username(Arena *arena)
{
    char *str = 0;
    DWORD size = 0;
	EXTENDED_NAME_FORMAT format = NameDisplay;
    if(!GetUserNameExA(format, NULL, &size) && GetLastError() == ERROR_MORE_DATA)
    {
		
        str = push_array(arena, char, size);
        if(GetUserNameExA(format, str, &size))
        {
            return  str8((u8 *)str, (u64)size);
        }
    }
	size = UNLEN + 1;
    str = push_array(arena, char, size);
    if(GetUserNameA(str, &size))
    {
        return str8((u8 *)str, (u64)size);
    }
    return str8_lit("unknown");
}

internal String8
os_get_user_path(void)
{
	char *user_profile = getenv("USERPROFILE");
	String8 result = str8_cstring(user_profile);
	return result;
}

internal b32
os_is_internet_connected()
{
	DWORD flag = {0};
	b32 result = (b32)InternetGetConnectedState(&flag, 0);
	return result;
}

////////////////////////////////
//~ rjf: @os_hooks GUIDs (Implemented Per-OS)

internal Guid
os_make_guid(void)
{
	Guid result; MemoryZeroStruct(&result);
	UUID uuid;
	RPC_STATUS rpc_status = UuidCreate(&uuid);
	if(rpc_status == RPC_S_OK)
	{
		result.data1 = uuid.Data1;
		result.data2 = uuid.Data2;
		result.data3 = uuid.Data3;
		MemoryCopyArray(result.data4, uuid.Data4);
	}
	return result;
}

//
//- rsb: Serial
//

//- rsb: https://learn.microsoft.com/en-us/previous-versions/ff802693(v=msdn.10)?redirectedfrom=MSDN
internal OS_Serial_Handle 
os_serial_open(String8Array port_array, u64 port_selected, Serial_Protocol protocol, Serial_Read_Timeouts read_timeouts, Serial_Write_Timeouts write_timeouts)
{
	//- rsb: open com port
	Temp temp = scratch_begin(0);
	String8 com_port = w32_hklm_read(temp.arena, str8_lit("HARDWARE\\DEVICEMAP\\SERIALCOMM"), port_array.v[port_selected]);
    String8 filename = push_str8_cat(temp.arena, str8_lit("\\\\.\\"), com_port);
	u64 com_port_id = u64_from_str8(str8_skip(com_port, 3), 10);
	HANDLE file = CreateFileA((char *)filename.str, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	scratch_end(temp);
	
	//- rsb: set protocol, timeouts
	OS_Serial_Handle handle = {0};
	if(file != INVALID_HANDLE_VALUE)
    {
        handle.os_handle.u64[0] = (U64)file;
		handle.port_selected = port_selected;
		handle.com_port_id = com_port_id;
		handle.connected = 1;
		
		BYTE stop_bits = ONESTOPBIT;
        switch(protocol.stop_bits)
        {
            case Serial_Stop_One:
            {
                stop_bits = ONESTOPBIT;
            } break;
            
            case Serial_Stop_One_Point_Five:
            {
                stop_bits = ONE5STOPBITS;
            } break;
            
            case Serial_Stop_Two:
            {
                stop_bits = TWOSTOPBITS;
            } break;
        };
        
        BYTE parity_bits = NOPARITY;
        switch(protocol.parity_bits)
        {
            case Serial_Parity_Even:
            {
                parity_bits = EVENPARITY;
            } break;
            
            case Serial_Parity_Odd:
            {
                parity_bits = ODDPARITY;
            } break;
            
            case Serial_Parity_No:
            {
                parity_bits = NOPARITY;
            } break;
        };
        
        DCB os_protocol = {};
        os_protocol.DCBlength = sizeof(DCB);
        os_protocol.BaudRate = protocol.baud_rate;
        os_protocol.fBinary = 1;
        os_protocol.fParity = (parity_bits == NOPARITY) ? 0 : 1;
        os_protocol.fOutxCtsFlow = 0;
        os_protocol.fOutxDsrFlow = 0;
        os_protocol.fDtrControl = DTR_CONTROL_DISABLE;
        os_protocol.fDsrSensitivity = 0;
        os_protocol.fTXContinueOnXoff = 0;
        os_protocol.fOutX = 0; 
        os_protocol.fInX = 0;
        os_protocol.fErrorChar = 1; 
        os_protocol.fNull = 0;
        os_protocol.fRtsControl = RTS_CONTROL_DISABLE;
        os_protocol.fAbortOnError = 0;
        os_protocol.fDummy2; 
        os_protocol.wReserved = 0;
        os_protocol.XonLim = 0; 
        os_protocol.XoffLim = 0;
        os_protocol.ByteSize = (BYTE) protocol.byte_size;
        os_protocol.Parity = parity_bits;
        os_protocol.StopBits = stop_bits;
        os_protocol.XonChar = 0;
        os_protocol.XoffChar = 0;
        os_protocol.ErrorChar = 0x7F;
        os_protocol.EofChar = 0x7F;
        os_protocol.EvtChar = 0x7F;
        os_protocol.wReserved1;
        SetCommState((HANDLE)handle.os_handle.u64[0], &os_protocol);
        
        //- rsb: Serial timeouts
        //- rsb: https://learn.microsoft.com/en-us/windows/win32/devio/time-outs
        //- rsb: https://learn.microsoft.com/en-us/windows/win32/devio/communications-errors
        //- rsb: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-clearcommerror
        //- rsb: https://learn.microsoft.com/en-us/windows/win32/devio/read-and-write-operations
        //- rsb: https://learn.microsoft.com/en-us/windows/win32/devio/communications-events
        //- rsb: Timeout = (num_bytes * multiplier) + constant
        COMMTIMEOUTS os_timeouts = {};
        os_timeouts.ReadIntervalTimeout = read_timeouts.ms_wait_next_byte;
        os_timeouts.ReadTotalTimeoutMultiplier = read_timeouts.ms_per_byte; 
        os_timeouts.ReadTotalTimeoutConstant = read_timeouts.ms_per_transaction;
        os_timeouts.WriteTotalTimeoutConstant = write_timeouts.ms_per_transaction; 
        os_timeouts.WriteTotalTimeoutMultiplier = write_timeouts.ms_per_byte;
        SetCommTimeouts((HANDLE)handle.os_handle.u64[0], &os_timeouts);
    }
    return handle;
}

internal String8Array
os_serial_get_devices(Arena *arena)
{
	Temp temp = scratch_begin(arena);
	String8List list = w32_hklm_list_keys(temp.arena, str8_lit("HARDWARE\\DEVICEMAP\\SERIALCOMM"));
    str8_list_push_front(temp.arena, &list, str8_lit(""));
	String8Array result = str8_array_from_list_copy(arena, &list);
	scratch_end(temp);
	return result;
}

internal void 
os_serial_close(OS_Serial_Handle *serial)
{
	if(!os_handle_match(serial->os_handle, os_handle_zero()))
	{
		Temp temp = scratch_begin(0);
		String8 filename = push_str8f(temp.arena, "\\\\.\\COM%llu", serial->com_port_id);
		CloseHandle((HANDLE)serial->os_handle.u64[0]);
		DeleteFileA((char *)filename.str);
		scratch_end(temp);
	}
	MemoryZeroStruct(serial);
}

internal b32 
os_serial_transmit(OS_Serial_Handle *serial, String8 data)
{
    DWORD bytes_written = 0;
    if(serial->connected)
    {
        serial->connected = (b32) WriteFile((HANDLE)serial->os_handle.u64[0], data.str, data.size, &bytes_written, NULL);
		if(!serial->connected)
		{
			os_serial_close(serial);
		}
    }
    b32 success = (serial->connected && (bytes_written == data.size));
    return success;
}

internal String8
os_serial_receive(Arena *arena, OS_Serial_Handle *serial, u64 num_bytes)
{
	DWORD bytes_read = 0;
	String8 result = {0};
	if(serial->connected)
	{
		u8 *buffer = push_array(arena, u8, num_bytes);
		serial->connected = (b32) ReadFile((HANDLE)serial->os_handle.u64[0], buffer, num_bytes, &bytes_read, NULL);
		if(!serial->connected)
		{
			os_serial_close(serial);
		}
		else
		{
			result = str8(buffer, bytes_read);
		}
	}
	return result;
}
