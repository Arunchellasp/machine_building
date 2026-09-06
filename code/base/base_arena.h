
#ifndef BASE_ARENA_H

struct Arena_Params
{
	u64 reserve_size;
	u64 commit_size;
};

#define ARENA_HEADER_SIZE 128
struct Arena
{
    u64 pos;
	u64 cmt;
	u64 res;
	
	u64 res_size;
	u64 cmt_size;
};
StaticAssert(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

global u64 memory_page_size = KB(4);
global u64 arena_default_reserve_size = MB(512);
global u64 arena_default_commit_size  = MB(512);

struct Temp
{
	Arena *arena;
	u32 pos;
};

////////////////////////////////
//~ rjf: Memory Operation Macros

#define MemoryCopy(dst, src, size)    memmove((dst), (src), (size))
#define MemorySet(dst, byte, size)    memset((dst), (byte), (size))
#define MemoryCompare(a, b, size)     memcmp((a), (b), (size))
#define MemoryStrlen(ptr)             strlen(ptr)

#define MemoryCopyStruct(d,s)  MemoryCopy((d),(s),sizeof(*(d)))
#define MemoryCopyArray(d,s)   MemoryCopy((d),(s),sizeof(d))
#define MemoryCopyTyped(d,s,c) MemoryCopy((d),(s),sizeof(*(d))*(c))

#define MemoryZero(s,z)       memset((s),0,(z))
#define MemoryZeroStruct(s)   MemoryZero((s),sizeof(*(s)))
#define MemoryZeroArray(a)    MemoryZero((a),sizeof(a))
#define MemoryZeroTyped(m,c)  MemoryZero((m),sizeof(*(m))*(c))

#define MemoryMatch(a,b,z)     (MemoryCompare((a),(b),(z)) == 0)
#define MemoryMatchStruct(a,b)  MemoryMatch((a),(b),sizeof(*(a)))
#define MemoryMatchArray(a,b)   MemoryMatch((a),(b),sizeof(a))

#define MemoryRead(T,p,e)    ( ((p)+sizeof(T)<=(e))?(*(T*)(p)):(0) )
#define MemoryConsume(T,p,e) ( ((p)+sizeof(T)<=(e))?((p)+=sizeof(T),*(T*)((p)-sizeof(T))):((p)=(e),0) )

//
//~ rsb: arena creation / destruction
//

#define arena_alloc_default() arena_alloc(Arena_Params{.reserve_size = arena_default_reserve_size, .commit_size = arena_default_commit_size})
internal Arena *arena_alloc(Arena_Params params);
internal void arena_release(Arena *arena);
internal void *arena_push(Arena *arena, u64 size, u64 align);
internal void arena_pop_to(Arena *arena, u64 pos);
internal u64 arena_pos(Arena *arena);

//
//~ rsb: arena push/pop helpers
//

internal void arena_clear(Arena *arena);
internal void arena_pop(Arena *arena, u64 amt);

//
//~ rsb: push helper macros
//

#define push_array_no_zero_aligned(arena, T, c, align) \
(T *)arena_push((arena), sizeof(T)*(c), (align))
#define push_array_aligned(arena, T, c, align) \
(T *)MemoryZero(push_array_no_zero_aligned(arena, T, c, align), sizeof(T)*(c))

#define push_array_no_zero(arena, T, c) \
(T *)push_array_no_zero_aligned(arena, T, c, Max(8, AlignOf(T)))
#define push_array(arena, T, c)\
(T *)push_array_aligned(arena, T, c, Max(8, AlignOf(T)))

//
//~ rsb: temporary arena scopes
//

internal Temp temp_begin(Arena *arena);
internal void temp_end(Temp temp);

#define BASE_ARENA_H
#endif //BASE_ARENA_H
