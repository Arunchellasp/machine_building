
#ifndef OS_CORE_H
#define OS_CORE_H

typedef struct OS_Handle OS_Handle;
struct OS_Handle
{
	U64 u64[1];
};

typedef struct OS_HandleNode OS_HandleNode;
struct OS_HandleNode
{
	OS_HandleNode *next;
	OS_Handle v;
};

typedef struct OS_HandleList OS_HandleList;
struct OS_HandleList
{
	OS_HandleNode *first;
	OS_HandleNode *last;
	U64 count;
};

typedef struct OS_HandleArray OS_HandleArray;
struct OS_HandleArray
{
	OS_Handle *v;
	U64 count;
};

typedef U32 OS_AccessFlags;
enum
{
	OS_AccessFlag_Read        = (1<<0),
	OS_AccessFlag_Write       = (1<<1),
	OS_AccessFlag_Execute     = (1<<2),
	OS_AccessFlag_Append      = (1<<3),
	OS_AccessFlag_ShareRead   = (1<<4),
	OS_AccessFlag_ShareWrite  = (1<<5),
	OS_AccessFlag_Inherited   = (1<<6),
};

typedef u32 OS_FileIterFlags;
enum
{
	OS_FileIterFlag_SkipFolders     = (1 << 0),
	OS_FileIterFlag_SkipFiles       = (1 << 1),
	OS_FileIterFlag_SkipHiddenFiles = (1 << 2),
	OS_FileIterFlag_Done            = (1 << 31),
};

struct OS_FileIter
{
	OS_FileIterFlags flags;
	u8 memory[800];
};

typedef u32 FilePropertyFlags;
enum
{
	FilePropertyFlag_IsFolder = (1 << 0),
};

enum WeekDay 
{
	WeekDay_Sun = 0,
	WeekDay_Mon,
	WeekDay_Tue,
	WeekDay_Wed,
	WeekDay_Thu,
	WeekDay_Fri,
	WeekDay_Sat,
	WeekDay_COUNT,
};

enum Month
{
	Month_Jan = 0,
	Month_Feb,
	Month_Mar,
	Month_Apr,
	Month_May,
	Month_Jun,
	Month_Jul,
	Month_Aug,
	Month_Sep,
	Month_Oct,
	Month_Nov,
	Month_Dec,
	Month_COUNT,
};

struct DateTime
{
	u16 micro_sec; // [0, 999]
	u16 msec; // [0, 999]
	u16 sec;  // [0, 59]
	u16 min;  // [0, 59]
	u16 hour; // [0, 23]
	u16 day;  // [1, 31]
	union
	{
		WeekDay week_day;
		u32 wday; // [0, 6]
	};
	union
	{
		Month month;
		u32 mon; // [0, 11]
	};
	u32 year;
};

typedef u64 DenseTime;

struct FileProperties
{
	u64 size;
	DenseTime modified;
	DenseTime created;
	FilePropertyFlags flags;
};

struct OS_FileInfo
{
	String8 name;
	FileProperties props;
};

struct OS_FileInfoNode
{
	OS_FileInfoNode *next;
	OS_FileInfo v;
};

struct OS_FileInfoList
{
	OS_FileInfoNode *first;
	OS_FileInfoNode *last;
	u64 count;
};

struct OS_FileInfoArray
{
	OS_FileInfo *v;
	u64 count;
};

struct OS_SystemInfo
{
	u64 allocation_granularity;
	u64 page_size;
	u64 logical_processor_count;
};

internal OS_SystemInfo *os_get_system_info(void);
internal void *os_reserve(u64 size);
internal b32 os_commit(void *ptr, u64 size);
internal void os_decommit(void *ptr, u64 size);
internal void os_release(void *ptr, u64 size);

typedef struct OS_FileID OS_FileID;
struct OS_FileID
{
	U64 v[3];
};

//- rsb: custom

struct OS_Button
{
	b32 pressed;
	i32 half_transition_count;
};

struct OS_Joystick
{
    b32 connected;
    i32 error;
    u32 packet_no;
    i32 press_count;
    union
    {
		struct
        {
            OS_Button up, down, left, right, start, back, lb, rb, a, b, x, y;
            r32 lx, ly, rx, ry; // x,y for left and right thumbsticks, normalized to [-1,1]
            u32 ltrigger, rtrigger; // 0 or 255 - no or yes
        };
        
        struct
        {
            OS_Button buttons[12];
            r32 analog[4];
            u32 triggers[2];
        } data;
    };
};

enum Serial_Parity_Bits
{
    Serial_Parity_Even = 0,
    Serial_Parity_Odd,
    Serial_Parity_No
};

enum Serial_Stop_Bits
{
    Serial_Stop_One = 0,
    Serial_Stop_One_Point_Five,
    Serial_Stop_Two
};

struct Serial_Protocol
{
    u32 baud_rate;
    u32 byte_size;
    Serial_Stop_Bits stop_bits;
    Serial_Parity_Bits parity_bits;
};

//- rsb: Timeout = (num_bytes * ms_per_byte)  + ms_per_transaction
struct Serial_Read_Timeouts
{
    u32 ms_wait_next_byte;
    u32 ms_per_byte;
    u32 ms_per_transaction;
};

struct Serial_Write_Timeouts
{
    u32 ms_per_byte;
    u32 ms_per_transaction;
};

struct OS_Serial_Handle
{
	u64 port_selected;
	u64 com_port_id;
    b32 connected;
    OS_Handle os_handle;
};

// TODO(rsb): Improve this???
//- rsb: file search 
struct FileSystemFilters
{
	OS_FileIterFlags flags;
	String8Array file_extensions;
	String8Array exclude_folders;
};

internal OS_FileInfoArray file_info_array_from_path(Arena *arena, String8 path, FileSystemFilters *filter);

////////////////////////////////
//~ rjf: Thread Types

typedef void OS_ThreadFunctionType(void *ptr);

////////////////////////////////
//~ rjf: @os_hooks File System (Implemented Per-OS)

//- rjf: files
internal OS_Handle      os_file_open(OS_AccessFlags flags, String8 path);
internal void           os_file_close(OS_Handle file);
internal U64            os_file_read(OS_Handle file, Rng1U64 rng, void *out_data);
#define os_file_read_struct(f, off, ptr) os_file_read((f), r1u64((off), (off)+sizeof(*(ptr))), (ptr))
internal U64            os_file_write(OS_Handle file, Rng1U64 rng, void *data);
internal B32            os_file_set_times(OS_Handle file, DateTime time);
internal FileProperties os_properties_from_file(OS_Handle file);
internal OS_FileID      os_id_from_file(OS_Handle file);
internal B32            os_file_reserve_size(OS_Handle file, U64 size);
internal B32            os_delete_file_at_path(String8 path);
internal B32            os_copy_file_path(String8 dst, String8 src);
internal B32            os_move_file_path(String8 dst, String8 src);
internal String8        os_full_path_from_path(Arena *arena, String8 path);
internal B32            os_file_path_exists(String8 path);
internal B32            os_folder_path_exists(String8 path);
internal FileProperties os_properties_from_file_path(String8 path);

////////////////////////////////
//~ rjf: Filesystem Helpers (Helpers, Implemented Once)

internal String8        os_data_from_file_path(Arena *arena, String8 path);
internal B32            os_write_data_to_file_path(String8 path, String8 data);
internal B32            os_write_data_list_to_file_path(String8 path, String8List list);
internal B32            os_append_data_to_file_path(String8 path, String8 data);
internal OS_FileID      os_id_from_file_path(String8 path);
internal S64            os_file_id_compare(OS_FileID a, OS_FileID b);
internal String8        os_string_from_file_range(Arena *arena, OS_Handle file, Rng1U64 range);

////////////////////////////////
//~ rjf: @os_hooks Synchronization Primitives (Implemented Per-OS)

//- rjf: recursive mutexes
internal OS_Handle os_mutex_alloc(void);
internal void      os_mutex_release(OS_Handle mutex);
internal void      os_mutex_take(OS_Handle mutex);
internal void      os_mutex_drop(OS_Handle mutex);

//- rjf: reader/writer mutexes
internal OS_Handle os_rw_mutex_alloc(void);
internal void      os_rw_mutex_release(OS_Handle rw_mutex);
internal void      os_rw_mutex_take_r(OS_Handle mutex);
internal void      os_rw_mutex_drop_r(OS_Handle mutex);
internal void      os_rw_mutex_take_w(OS_Handle mutex);
internal void      os_rw_mutex_drop_w(OS_Handle mutex);

//- rjf: condition variables
internal OS_Handle os_condition_variable_alloc(void);
internal void      os_condition_variable_release(OS_Handle cv);
// returns false on timeout, true on signal, (max_wait_ms = max_U64) -> no timeout
internal B32       os_condition_variable_wait(OS_Handle cv, OS_Handle mutex, U64 endt_us);
internal B32       os_condition_variable_wait_rw_r(OS_Handle cv, OS_Handle mutex_rw, U64 endt_us);
internal B32       os_condition_variable_wait_rw_w(OS_Handle cv, OS_Handle mutex_rw, U64 endt_us);
internal void      os_condition_variable_signal(OS_Handle cv);
internal void      os_condition_variable_broadcast(OS_Handle cv);

//- rjf: cross-process semaphores
internal OS_Handle os_semaphore_alloc(U32 initial_count, U32 max_count, String8 name);
internal void      os_semaphore_release(OS_Handle semaphore);
internal OS_Handle os_semaphore_open(String8 name);
internal void      os_semaphore_close(OS_Handle semaphore);
internal B32       os_semaphore_take(OS_Handle semaphore, U64 endt_us);
internal void      os_semaphore_drop(OS_Handle semaphore);

//- rjf: scope macros
#define OS_MutexScope(mutex) DeferLoop(os_mutex_take(mutex), os_mutex_drop(mutex))
#define OS_MutexScopeR(mutex) DeferLoop(os_rw_mutex_take_r(mutex), os_rw_mutex_drop_r(mutex))
#define OS_MutexScopeW(mutex) DeferLoop(os_rw_mutex_take_w(mutex), os_rw_mutex_drop_w(mutex))
#define OS_MutexScopeRWPromote(mutex) DeferLoop((os_rw_mutex_drop_r(mutex), os_rw_mutex_take_w(mutex)), (os_rw_mutex_drop_w(mutex), os_rw_mutex_take_r(mutex)))

#endif //OS_CORE_H
