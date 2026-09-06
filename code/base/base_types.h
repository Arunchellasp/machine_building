#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i08;
typedef int8_t i8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u08;
typedef uint8_t u8;

typedef size_t mxx;

typedef i64 s64;
typedef i32 s32;
typedef i16 s16;
typedef i08 s08;
typedef i8 s8;

typedef i64 b64;
typedef i32 b32;
typedef i16 b16;
typedef i08 b08;
typedef i8 b8;

typedef float r32;
typedef double r64;

typedef r32 f32;
typedef r64 f64;

typedef i64 I64;
typedef i32 I32;
typedef i16 I16;
typedef i08 I08;
typedef i8 I8;

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef S8       B8;
typedef S16      B16;
typedef S32      B32;
typedef S64      B64;
typedef float    F32;
typedef double   F64;
typedef void VoidProc(void);

typedef union U128 U128;
union U128
{
	U8 u8[16];
	U16 u16[8];
	U32 u32[4];
	U64 u64[2];
};
typedef union U256 U256;
union U256
{
	U8 u8[32];
	U16 u16[16];
	U32 u32[8];
	U64 u64[4];
	U128 u128[2];
};
typedef union U512 U512;
union U512
{
	U8 u8[64];
	U16 u16[32];
	U32 u32[16];
	U64 u64[8];
	U128 u128[4];
	U256 u256[2];
};

#define internal static
#define local_persist static
#define global static

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

#define Glue_(A,B) A##B
#define Glue(A,B) Glue_(A,B)

#define StaticAssert(exp, id) global u8 Glue(id, __LINE__)[(exp)?1:-1]
#define Assert(exp) {if(!(exp)){*(int *)0 = 0;}}

#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))

#define Clamp(A,X,B)  (((X)<(A))?(A):((X)>(B))?(B):(X))
#define ClampBot(A,X) Max(A,X)
#define ClampTop(X,B) Min(X,B)

#define Swap(T,a,b) do{T t__ = a; a = b; b = t__;}while(0)

#define ArrayCount(arr) (sizeof(arr) / sizeof(arr[0]))
#define Member(T, m) (((T*)0)->m)
#define OffsetOf(T, m) ((u64)(&Member(T, m)))
#define CastFromMember(T, m, ptr) (T*)(((u8 *)ptr) - OffsetOf(T, m))
#define InvalidCodePath Assert(!"Invalid code path")
#define InvalidDefaultCase default: {InvalidCodePath;} break;
#define IntFromPtr(ptr) ((U64)(ptr))
#define PtrFromInt(i) (void*)((U8*)0 + (i))

#include <intrin.h>
#define ins_atomic_u64_eval(x)                 *((volatile U64 *)(x))
#define ins_atomic_u64_inc_eval(x)             InterlockedIncrement64((volatile __int64 *)(x))
#define ins_atomic_u64_dec_eval(x)             InterlockedDecrement64((volatile __int64 *)(x))
#define ins_atomic_u64_eval_assign(x,c)        InterlockedExchange64((volatile __int64 *)(x),(c))
#define ins_atomic_u64_add_eval(x,c)           InterlockedAdd64((volatile __int64 *)(x), c)
#define ins_atomic_u64_eval_cond_assign(x,k,c) InterlockedCompareExchange64((volatile __int64 *)(x),(k),(c))
#define ins_atomic_u32_eval(x)                 *((volatile U32 *)(x))
#define ins_atomic_u32_inc_eval(x)             InterlockedIncrement((volatile LONG *)x)
#define ins_atomic_u32_eval_assign(x,c)        InterlockedExchange((volatile LONG *)(x),(c))
#define ins_atomic_u32_eval_cond_assign(x,k,c) InterlockedCompareExchange((volatile LONG *)(x),(k),(c))
#define ins_atomic_u32_add_eval(x,c)           InterlockedAdd((volatile LONG *)(x), c)

#define FOURCC(string) (((u32)(string[0]) << 0) | ((u32)(string[1]) << 8) | ((u32)(string[2]) << 16) | ((u32)(string[3]) << 24))

#if _MSC_VER
#define COMPILER_MSVC 1
#endif

#if defined(COMPILER_MSVC)
# pragma section(".rdata$", read)
#define read_only __declspec(allocate(".rdata$"))
#endif

////////////////////////////////
//~ rjf: Basic Types & Spaces

typedef enum Dimension
{
	Dimension_X,
	Dimension_Y,
	Dimension_Z,
	Dimension_W,
}
Dimension;

typedef enum Side
{
	Side_Invalid = -1,
	Side_Min,
	Side_Max,
	Side_COUNT,
}
Side;
#define side_flip(s) ((Side)(!(s)))

typedef enum Axis2
{
	Axis2_Invalid = -1,
	Axis2_X,
	Axis2_Y,
	Axis2_COUNT,
}
Axis2;
#define axis2_flip(a) ((Axis2)(!(a)))

typedef enum Corner
{
	Corner_Invalid = -1,
	Corner_00,
	Corner_01,
	Corner_10,
	Corner_11,
	Corner_COUNT
}
Corner;

typedef enum Dir2
{
	Dir2_Invalid = -1,
	Dir2_Left,
	Dir2_Up,
	Dir2_Right,
	Dir2_Down,
	Dir2_COUNT
}
Dir2;
#define axis2_from_dir2(d) (((d) & 1) ? Axis2_Y : Axis2_X)
#define side_from_dir2(d) (((d) < Dir2_Right) ? Side_Min : Side_Max)

////////////////////////////////
//~ Globally Unique Ids

typedef union Guid Guid;
union Guid
{
	struct
	{
		U32 data1;
		U16 data2;
		U16 data3;
		U8  data4[8];
	};
	U8 v[16];
};
StaticAssert(sizeof(Guid) == 16, g_guid_size_check);

////////////////////////////////
//~ rjf: Units

#define KB(n)  (((U64)(n)) << 10)
#define MB(n)  (((U64)(n)) << 20)
#define GB(n)  (((U64)(n)) << 30)
#define TB(n)  (((U64)(n)) << 40)
#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

#define PI 3.14159265359f

#define Compose64Bit(a,b)  ((((U64)a) << 32) | ((U64)b));
#define AlignOf(T)      __alignof(T)
#define AlignPow2(x,b)  (((x) + (b) - 1)&(~((b) - 1)))
#define IsPow2(x)       ((x)!=0 && ((x)&((x)-1))==0)
#define IsPow2OrZero(x) (((x)&((x)-1))==0)

#define DeferIt(it) Glue(it, __LINE__)
#define DeferLoop(begin, end) for(int DeferIt(_i_) = ((begin), 0); !DeferIt(_i_); DeferIt(_i_) += 1, (end))
#define DeferLoopCheckedBegin(begin, end) if(begin) DeferLoop(0, end)

global U64 max_U64 = 0xffffffffffffffffull;
global U32 max_U32 = 0xffffffff;
global U16 max_U16 = 0xffff;
global U8  max_U8  = 0xff;

global S64 max_S64 = (S64)0x7fffffffffffffffull;
global S32 max_S32 = (S32)0x7fffffff;
global S16 max_S16 = (S16)0x7fff;
global S8  max_S8  =  (S8)0x7f;

global S64 min_S64 = (S64)0xffffffffffffffffull;
global S32 min_S32 = (S32)0xffffffff;
global S16 min_S16 = (S16)0xffff;
global S8  min_S8  =  (S8)0xff;

global const U32 bitmask1  = 0x00000001;
global const U32 bitmask2  = 0x00000003;
global const U32 bitmask3  = 0x00000007;
global const U32 bitmask4  = 0x0000000f;
global const U32 bitmask5  = 0x0000001f;
global const U32 bitmask6  = 0x0000003f;
global const U32 bitmask7  = 0x0000007f;
global const U32 bitmask8  = 0x000000ff;
global const U32 bitmask9  = 0x000001ff;
global const U32 bitmask10 = 0x000003ff;
global const U32 bitmask11 = 0x000007ff;
global const U32 bitmask12 = 0x00000fff;
global const U32 bitmask13 = 0x00001fff;
global const U32 bitmask14 = 0x00003fff;
global const U32 bitmask15 = 0x00007fff;
global const U32 bitmask16 = 0x0000ffff;
global const U32 bitmask17 = 0x0001ffff;
global const U32 bitmask18 = 0x0003ffff;
global const U32 bitmask19 = 0x0007ffff;
global const U32 bitmask20 = 0x000fffff;
global const U32 bitmask21 = 0x001fffff;
global const U32 bitmask22 = 0x003fffff;
global const U32 bitmask23 = 0x007fffff;
global const U32 bitmask24 = 0x00ffffff;
global const U32 bitmask25 = 0x01ffffff;
global const U32 bitmask26 = 0x03ffffff;
global const U32 bitmask27 = 0x07ffffff;
global const U32 bitmask28 = 0x0fffffff;
global const U32 bitmask29 = 0x1fffffff;
global const U32 bitmask30 = 0x3fffffff;
global const U32 bitmask31 = 0x7fffffff;
global const U32 bitmask32 = 0xffffffff;

global const U64 bitmask33 = 0x00000001ffffffffull;
global const U64 bitmask34 = 0x00000003ffffffffull;
global const U64 bitmask35 = 0x00000007ffffffffull;
global const U64 bitmask36 = 0x0000000fffffffffull;
global const U64 bitmask37 = 0x0000001fffffffffull;
global const U64 bitmask38 = 0x0000003fffffffffull;
global const U64 bitmask39 = 0x0000007fffffffffull;
global const U64 bitmask40 = 0x000000ffffffffffull;
global const U64 bitmask41 = 0x000001ffffffffffull;
global const U64 bitmask42 = 0x000003ffffffffffull;
global const U64 bitmask43 = 0x000007ffffffffffull;
global const U64 bitmask44 = 0x00000fffffffffffull;
global const U64 bitmask45 = 0x00001fffffffffffull;
global const U64 bitmask46 = 0x00003fffffffffffull;
global const U64 bitmask47 = 0x00007fffffffffffull;
global const U64 bitmask48 = 0x0000ffffffffffffull;
global const U64 bitmask49 = 0x0001ffffffffffffull;
global const U64 bitmask50 = 0x0003ffffffffffffull;
global const U64 bitmask51 = 0x0007ffffffffffffull;
global const U64 bitmask52 = 0x000fffffffffffffull;
global const U64 bitmask53 = 0x001fffffffffffffull;
global const U64 bitmask54 = 0x003fffffffffffffull;
global const U64 bitmask55 = 0x007fffffffffffffull;
global const U64 bitmask56 = 0x00ffffffffffffffull;
global const U64 bitmask57 = 0x01ffffffffffffffull;
global const U64 bitmask58 = 0x03ffffffffffffffull;
global const U64 bitmask59 = 0x07ffffffffffffffull;
global const U64 bitmask60 = 0x0fffffffffffffffull;
global const U64 bitmask61 = 0x1fffffffffffffffull;
global const U64 bitmask62 = 0x3fffffffffffffffull;
global const U64 bitmask63 = 0x7fffffffffffffffull;
global const U64 bitmask64 = 0xffffffffffffffffull;

global const U32 bit1  = (1<<0);
global const U32 bit2  = (1<<1);
global const U32 bit3  = (1<<2);
global const U32 bit4  = (1<<3);
global const U32 bit5  = (1<<4);
global const U32 bit6  = (1<<5);
global const U32 bit7  = (1<<6);
global const U32 bit8  = (1<<7);
global const U32 bit9  = (1<<8);
global const U32 bit10 = (1<<9);
global const U32 bit11 = (1<<10);
global const U32 bit12 = (1<<11);
global const U32 bit13 = (1<<12);
global const U32 bit14 = (1<<13);
global const U32 bit15 = (1<<14);
global const U32 bit16 = (1<<15);
global const U32 bit17 = (1<<16);
global const U32 bit18 = (1<<17);
global const U32 bit19 = (1<<18);
global const U32 bit20 = (1<<19);
global const U32 bit21 = (1<<20);
global const U32 bit22 = (1<<21);
global const U32 bit23 = (1<<22);
global const U32 bit24 = (1<<23);
global const U32 bit25 = (1<<24);
global const U32 bit26 = (1<<25);
global const U32 bit27 = (1<<26);
global const U32 bit28 = (1<<27);
global const U32 bit29 = (1<<28);
global const U32 bit30 = (1<<29);
global const U32 bit31 = (1<<30);
global const U32 bit32 = (1<<31);

global const U64 bit33 = (1ull<<32);
global const U64 bit34 = (1ull<<33);
global const U64 bit35 = (1ull<<34);
global const U64 bit36 = (1ull<<35);
global const U64 bit37 = (1ull<<36);
global const U64 bit38 = (1ull<<37);
global const U64 bit39 = (1ull<<38);
global const U64 bit40 = (1ull<<39);
global const U64 bit41 = (1ull<<40);
global const U64 bit42 = (1ull<<41);
global const U64 bit43 = (1ull<<42);
global const U64 bit44 = (1ull<<43);
global const U64 bit45 = (1ull<<44);
global const U64 bit46 = (1ull<<45);
global const U64 bit47 = (1ull<<46);
global const U64 bit48 = (1ull<<47);
global const U64 bit49 = (1ull<<48);
global const U64 bit50 = (1ull<<49);
global const U64 bit51 = (1ull<<50);
global const U64 bit52 = (1ull<<51);
global const U64 bit53 = (1ull<<52);
global const U64 bit54 = (1ull<<53);
global const U64 bit55 = (1ull<<54);
global const U64 bit56 = (1ull<<55);
global const U64 bit57 = (1ull<<56);
global const U64 bit58 = (1ull<<57);
global const U64 bit59 = (1ull<<58);
global const U64 bit60 = (1ull<<59);
global const U64 bit61 = (1ull<<60);
global const U64 bit62 = (1ull<<61);
global const U64 bit63 = (1ull<<62);
global const U64 bit64 = (1ull<<63);

////////////////////////////////
//~ rjf: Safe Casts

internal U16
safe_cast_u16(U32 x)
{
	Assert(x <= max_U16);
	U16 result = (U16)x;
	return result;
}

internal U32
safe_cast_u32(U64 x)
{
	Assert(x <= max_U32);
	U32 result = (U32)x;
	return result;
}

internal S32
safe_cast_s32(S64 x)
{
	Assert(x <= max_S32);
	S32 result = (S32)x;
	return result;
}

////////////////////////////////
//~ rjf: Linked List Building Macros

//- rjf: linked list macro helpers
#define CheckNil(nil,p) ((p) == 0 || (p) == nil)
#define SetNil(nil,p) ((p) = nil)

//- rjf: doubly-linked-lists
#define DLLInsert_NPZ(nil,f,l,p,n,next,prev) (CheckNil(nil,f) ? \
((f) = (l) = (n), SetNil(nil,(n)->next), SetNil(nil,(n)->prev)) :\
CheckNil(nil,p) ? \
((n)->next = (f), (f)->prev = (n), (f) = (n), SetNil(nil,(n)->prev)) :\
((p)==(l)) ? \
((l)->next = (n), (n)->prev = (l), (l) = (n), SetNil(nil, (n)->next)) :\
(((!CheckNil(nil,p) && CheckNil(nil,(p)->next)) ? (0) : ((p)->next->prev = (n))), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define DLLPushBack_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,f,l,l,n,next,prev)
#define DLLPushFront_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,l,f,f,n,prev,next)
#define DLLRemove_NPZ(nil,f,l,n,next,prev) (((n) == (f) ? (f) = (n)->next : (0)),\
((n) == (l) ? (l) = (l)->prev : (0)),\
(CheckNil(nil,(n)->prev) ? (0) :\
((n)->prev->next = (n)->next)),\
(CheckNil(nil,(n)->next) ? (0) :\
((n)->next->prev = (n)->prev)))

//- rjf: singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((l)->next=(n),(l)=(n),SetNil(nil,(n)->next)))
#define SLLQueuePushFront_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePop_NZ(nil,f,l,next) ((f)==(l)?\
(SetNil(nil,f),SetNil(nil,l)):\
((f)=(f)->next))

//- rjf: singly-linked, singly-headed lists (stacks)
#define SLLStackPush_N(f,n,next) ((n)->next=(f), (f)=(n))
#define SLLStackPop_N(f,next) ((f)=(f)->next)

//- rjf: doubly-linked-list helpers
#define DLLInsert_NP(f,l,p,n,next,prev) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack_NP(f,l,n,next,prev) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront_NP(f,l,n,next,prev) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove_NP(f,l,n,next,prev) DLLRemove_NPZ(0,f,l,n,next,prev)
#define DLLInsert(f,l,p,n) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack(f,l,n) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront(f,l,n) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove(f,l,n) DLLRemove_NPZ(0,f,l,n,next,prev)

//- rjf: singly-linked, doubly-headed list helpers
#define SLLQueuePush_N(f,l,n,next) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront_N(f,l,n,next) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop_N(f,l,next) SLLQueuePop_NZ(0,f,l,next)
#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop(f,l) SLLQueuePop_NZ(0,f,l,next)

//- rjf: singly-linked, singly-headed list helpers
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)
#define SLLStackPop(f) SLLStackPop_N(f,next)

#if 0
{
	//- rsb: f,l,n = first, last, node
	//- rsb: SLLStackPush
	n->next = f;
	f = n;
	
	//- rsb: SLLStackPop
	if(f == 0)
	{
		
	}
	else
	{
		f = f->next;
	}
	
	//- rsb: SLLQueuePush
	if(f == 0)
	{
		f = l = n;
		n->next = 0;
	}
	else
	{
		l->next = n;
		l = n;
		n->next = 0;
	}
	
	//- rsb: SLLQueuePushFront
	if(f == 0)
	{
		n->next = 0;
		f = l = n;
	}
	else
	{
		n->next = f;
		f = n;
	}
	
	//- rsb: SLLQueuePop
	if(f == l)
	{
		f = l = 0;
	}
	else
	{
		f = f->next;
	}
}
#endif

internal U32
u32_from_u64_saturate(U64 x){
	U32 x32 = (x > max_U32)?max_U32:(U32)x;
	return(x32);
}

internal U64
u64_up_to_pow2(U64 x){
	if (x == 0){
		x = 1;
	}
	else{
		x -= 1;
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		x |= (x >> 32);
		x += 1;
	}
	return(x);
}

internal S32
extend_sign32(U32 x, U32 size){
	U32 high_bit = size * 8;
	U32 shift = 32 - high_bit;
	S32 result = ((S32)x << shift) >> shift;
	return result;
}

internal S64
extend_sign64(U64 x, U64 size){
	U64 high_bit = size * 8;
	U64 shift = 64 - high_bit;
	S64 result = ((S64)x << shift) >> shift;
	return result;
}

internal U16
bswap_u16(U16 x)
{
	U16 result = (((x & 0xFF00) >> 8) |
				  ((x & 0x00FF) << 8));
	return result;
}

internal U32
bswap_u32(U32 x)
{
	U32 result = (((x & 0xFF000000) >> 24) |
				  ((x & 0x00FF0000) >> 8)  |
				  ((x & 0x0000FF00) << 8)  |
				  ((x & 0x000000FF) << 24));
	return result;
}

internal U64
bswap_u64(U64 x)
{
	// TODO(nick): naive bswap, replace with something that is faster like an intrinsic
	U64 result = (((x & 0xFF00000000000000ULL) >> 56) |
				  ((x & 0x00FF000000000000ULL) >> 40) |
				  ((x & 0x0000FF0000000000ULL) >> 24) |
				  ((x & 0x000000FF00000000ULL) >> 8)  |
				  ((x & 0x00000000FF000000ULL) << 8)  |
				  ((x & 0x0000000000FF0000ULL) << 24) |
				  ((x & 0x000000000000FF00ULL) << 40) |
				  ((x & 0x00000000000000FFULL) << 56));
	return result;
}

#endif //BASE_TYPES_H
