#ifndef BASE_MATH_H
#define BASE_MATH_H

// TODO(rsb): use full raddebugger base api

#define radians_from_turns_f64(v) ((v)*2*3.1415926535897)
#define turns_from_radians_f64(v) ((v)/(2*3.1415926535897))
#define degrees_from_turns_f64(v) ((v)*360.0)
#define turns_from_degrees_f64(v) ((v)/360.0)
#define degrees_from_radians_f64(v) (degrees_from_turns_f64(turns_from_radians_f64(v)))
#define radians_from_degrees_f64(v) (radians_from_turns_f64(turns_from_degrees_f64(v)))

#define abs_s32(v) abs(v)
#define abs_s64(v) (S64)llabs(v)
#define abs_f64(v)    fabs(v)
#define sqrt_f64(v) sqrt(v)
#define pow_f64(b, e) pow((b), (e))
#define atan2_f64(dy,dx) atan2(dy,dx)

#define radians_from_turns_f32(v) ((v)*2*3.1415926535897f)
#define turns_from_radians_f32(v) ((v)/(2*3.1415926535897f))
#define degrees_from_turns_f32(v) ((v)*360.f)
#define turns_from_degrees_f32(v) ((v)/360.f)
#define degrees_from_radians_f32(v) (degrees_from_turns_f32(turns_from_radians_f32(v)))
#define radians_from_degrees_f32(v) (radians_from_turns_f32(turns_from_degrees_f32(v)))
#define sqrt_f32(v)   sqrtf(v)
#define mod_f32(a, b) fmodf((a), (b))
#define pow_f32(b, e) powf((b), (e))
#define ceil_f32(v)   ceilf(v)
#define floor_f32(v)  floorf(v)
#define round_f32(v)  roundf(v)
#define abs_f32(v)    fabsf(v)

typedef union Rng1U64 Rng1U64;
union Rng1U64
{
	struct
	{
		U64 min;
		U64 max;
	};
	U64 v[2];
};

union Vec3u8
{
	struct
	{
		u8 b;
		u8 g;
		u8 r;
	};
	u8 v[3];
};

union Vec2u64
{
	struct
	{
		u64 x;
		u64 y;
	};
	struct
	{
		u64 col;
		u64 row;
	};
	u64 v[2];
};

union Rng1u64
{
	struct
	{
		u64 min;
		u64 max;
	};
	u64 v[2];
};

union Rng1i32
{
	struct
	{
		i32 min;
		i32 max;
	};
	i32 v[2];
};

typedef union Vec2S32 Vec2S32;
union Vec2S32
{
	struct
	{
		S32 x;
		S32 y;
	};
	S32 v[2];
};

typedef union Vec2F32 Vec2F32;
union Vec2F32
{
	struct
	{
		F32 x;
		F32 y;
	};
	F32 v[2];
};

union V2i{
    struct {
        i32 x;
        i32 y;
    };
    
    struct{
        i32 width;
        i32 height;
    };
    
    struct {
        i32 cols;
        i32 rows;
    };
    
    i32 E[2];
};

struct Limits
{
    r32 min;
    r32 max;
};

struct I64Array
{
	i64 *val;
	u64 count;
};


typedef union Rng2S32 Rng2S32;
union Rng2S32
{
	struct
	{
		Vec2S32 min;
		Vec2S32 max;
	};
	struct
	{
		Vec2S32 p0;
		Vec2S32 p1;
	};
	struct
	{
		S32 x0;
		S32 y0;
		S32 x1;
		S32 y1;
	};
	Vec2S32 v[2];
};

//- rjf: 3-vectors

typedef union Vec3F32 Vec3F32;
union Vec3F32
{
	struct
	{
		F32 x;
		F32 y;
		F32 z;
	};
	struct
	{
		Vec2F32 xy;
		F32 _z0;
	};
	struct
	{
		F32 _x0;
		Vec2F32 yz;
	};
	F32 v[3];
};

#define v3f32(x, y, z) vec_3f32((x), (y), (z))
internal Vec3F32 vec_3f32(F32 x, F32 y, F32 z);
internal Vec3F32 add_3f32(Vec3F32 a, Vec3F32 b);
internal Vec3F32 sub_3f32(Vec3F32 a, Vec3F32 b);
internal Vec3F32 mul_3f32(Vec3F32 a, Vec3F32 b);
internal Vec3F32 div_3f32(Vec3F32 a, Vec3F32 b);
internal Vec3F32 scale_3f32(Vec3F32 v, F32 s);
internal F32 dot_3f32(Vec3F32 a, Vec3F32 b);
internal F32 length_squared_3f32(Vec3F32 v);
internal F32 length_3f32(Vec3F32 v);
internal Vec3F32 normalize_3f32(Vec3F32 v);
internal Vec3F32 cross_3f32(Vec3F32 a, Vec3F32 b);

typedef union Vec3S32 Vec3S32;
union Vec3S32
{
	struct
	{
		S32 x;
		S32 y;
		S32 z;
	};
	struct
	{
		Vec2S32 xy;
		S32 _z0;
	};
	struct
	{
		S32 _x0;
		Vec2S32 yz;
	};
	S32 v[3];
};

#define v3s32(x, y, z) vec_3s32((x), (y), (z))
internal Vec3S32 vec_3s32(S32 x, S32 y, S32 z);
internal Vec3S32 add_3s32(Vec3S32 a, Vec3S32 b);
internal Vec3S32 sub_3s32(Vec3S32 a, Vec3S32 b);
internal Vec3S32 mul_3s32(Vec3S32 a, Vec3S32 b);
internal Vec3S32 div_3s32(Vec3S32 a, Vec3S32 b);
internal Vec3S32 scale_3s32(Vec3S32 v, S32 s);
internal S32 dot_3s32(Vec3S32 a, Vec3S32 b);
internal S32 length_squared_3s32(Vec3S32 v);
internal S32 length_3s32(Vec3S32 v);
internal Vec3S32 normalize_3s32(Vec3S32 v);
internal Vec3S32 cross_3s32(Vec3S32 a, Vec3S32 b);

#define r1u64(min, max) rng_1u64((min), (max))
internal Rng1U64 rng_1u64(U64 min, U64 max);
internal Rng1U64 shift_1u64(Rng1U64 r, U64 x);
internal Rng1U64 pad_1u64(Rng1U64 r, U64 x);
internal U64 center_1u64(Rng1U64 r);
internal B32 contains_1u64(Rng1U64 r, U64 x);
internal U64 dim_1u64(Rng1U64 r);
internal Rng1U64 union_1u64(Rng1U64 a, Rng1U64 b);
internal Rng1U64 intersect_1u64(Rng1U64 a, Rng1U64 b);
internal U64 clamp_1u64(Rng1U64 r, U64 v);

#define r2s32(min, max) rng_2s32((min), (max))
#define r2s32p(x, y, z, w) r2s32(v2s32((x), (y)), v2s32((z), (w)))
internal Rng2S32 rng_2s32(Vec2S32 min, Vec2S32 max);
internal Rng2S32 shift_2s32(Rng2S32 r, Vec2S32 x);
internal Rng2S32 pad_2s32(Rng2S32 r, S32 x);
internal Vec2S32 center_2s32(Rng2S32 r);
internal B32 contains_2s32(Rng2S32 r, Vec2S32 x);
internal Vec2S32 dim_2s32(Rng2S32 r);
internal Rng2S32 union_2s32(Rng2S32 a, Rng2S32 b);
internal Rng2S32 intersect_2s32(Rng2S32 a, Rng2S32 b);
internal Vec2S32 clamp_2s32(Rng2S32 r, Vec2S32 v);


typedef union Rng2F32 Rng2F32;
union Rng2F32
{
	struct
	{
		Vec2F32 min;
		Vec2F32 max;
	};
	struct
	{
		Vec2F32 p0;
		Vec2F32 p1;
	};
	struct
	{
		F32 x0;
		F32 y0;
		F32 x1;
		F32 y1;
	};
	Vec2F32 v[2];
};
#define v2s32(x, y) vec_2s32((x), (y))
#define v2f32(x, y) vec_2f32((x), (y))

#define r2f32(min, max) rng_2f32((min), (max))
#define r2f32p(x, y, z, w) r2f32(v2f32((x), (y)), v2f32((z), (w)))
internal Rng2F32 rng_2f32(Vec2F32 min, Vec2F32 max);
internal Rng2F32 shift_2f32(Rng2F32 r, Vec2F32 x);
internal Rng2F32 pad_2f32(Rng2F32 r, F32 x);
internal Vec2F32 center_2f32(Rng2F32 r);
internal B32 contains_2f32(Rng2F32 r, Vec2F32 x);
internal Vec2F32 dim_2f32(Rng2F32 r);
internal Rng2F32 union_2f32(Rng2F32 a, Rng2F32 b);
internal Rng2F32 intersect_2f32(Rng2F32 a, Rng2F32 b);
internal Vec2F32 clamp_2f32(Rng2F32 r, Vec2F32 v);

//- rjf: 4-vectors

typedef union Vec4F32 Vec4F32;
union Vec4F32
{
	struct
	{
		F32 x;
		F32 y;
		F32 z;
		F32 w;
	};
	struct
	{
		Vec2F32 xy;
		Vec2F32 zw;
	};
	F32 v[4];
};

union Line
{
    struct
    {
        r32 slope;
        union
        {
            r32 intercept;
            r32 offset;
        };
    };
    struct
    {
        r32 m;
        r32 c;
    };
    struct
    {
        r32 mean;
        r32 variance;
    };
};

#define v4f32(x, y, z, w) vec_4f32((x), (y), (z), (w))
internal Vec4F32 vec_4f32(F32 x, F32 y, F32 z, F32 w);
internal Vec4F32 add_4f32(Vec4F32 a, Vec4F32 b);
internal Vec4F32 sub_4f32(Vec4F32 a, Vec4F32 b);
internal Vec4F32 mul_4f32(Vec4F32 a, Vec4F32 b);
internal Vec4F32 div_4f32(Vec4F32 a, Vec4F32 b);
internal Vec4F32 scale_4f32(Vec4F32 v, F32 s);
internal F32 dot_4f32(Vec4F32 a, Vec4F32 b);
internal F32 length_squared_4f32(Vec4F32 v);
internal F32 length_4f32(Vec4F32 v);
internal Vec4F32 normalize_4f32(Vec4F32 v);
internal Vec4F32 mix_4f32(Vec4F32 a, Vec4F32 b, F32 t);

typedef struct Mat3x3F32 Mat3x3F32;
struct Mat3x3F32
{
	F32 v[3][3];
};

inline internal Line init_conversion(r32 slope, r32 offset);

#endif //BASE_MATH_H
