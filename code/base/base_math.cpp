
//~ Vec ops
internal Vec2S32 vec_2s32(S32 x, S32 y)                         {Vec2S32 v = {x, y}; return v;}
internal Vec2S32 add_2s32(Vec2S32 a, Vec2S32 b)                 {Vec2S32 c = {a.x+b.x, a.y+b.y}; return c;}
internal Vec2S32 sub_2s32(Vec2S32 a, Vec2S32 b)                 {Vec2S32 c = {a.x-b.x, a.y-b.y}; return c;}
internal Vec2S32 mul_2s32(Vec2S32 a, Vec2S32 b)                 {Vec2S32 c = {a.x*b.x, a.y*b.y}; return c;}
internal Vec2S32 div_2s32(Vec2S32 a, Vec2S32 b)                 {Vec2S32 c = {a.x/b.x, a.y/b.y}; return c;}
internal Vec2S32 scale_2s32(Vec2S32 v, S32 s)                   {Vec2S32 c = {v.x*s, v.y*s}; return c;}
internal S32 min_2s32(Vec2S32 a)                                {S32 c = Min(a.x, a.y); return c;}
internal S32 dot_2s32(Vec2S32 a, Vec2S32 b)                     {S32 c = a.x*b.x + a.y*b.y; return c;}
internal S32 length_squared_2s32(Vec2S32 v)                     {S32 c = v.x*v.x + v.y*v.y; return c;}
internal S32 length_2s32(Vec2S32 v)                             {S32 c = (S32)sqrt_f32((F32)v.x*(F32)v.x + (F32)v.y*(F32)v.y); return c;}
internal Vec2S32 normalize_2s32(Vec2S32 v)                      {v = scale_2s32(v, (S32)(1.f/length_2s32(v))); return v;}
internal b32 match_2s32(Vec2S32 a, Vec2S32 b)                   {b32 result = (a.x == b.x) && (a.y == b.y); return result;}
internal b32 less_than_2s32(Vec2S32 a, Vec2S32 b)               {b32 result = (a.x < b.x) && (a.y < b.y); return result;}

internal Vec2F32 vec_2f32(F32 x, F32 y)                         {Vec2F32 v = {x, y}; return v;}
internal Vec2F32 add_2f32(Vec2F32 a, Vec2F32 b)                 {Vec2F32 c = {a.x+b.x, a.y+b.y}; return c;}
internal Vec2F32 sub_2f32(Vec2F32 a, Vec2F32 b)                 {Vec2F32 c = {a.x-b.x, a.y-b.y}; return c;}
internal Vec2F32 mul_2f32(Vec2F32 a, Vec2F32 b)                 {Vec2F32 c = {a.x*b.x, a.y*b.y}; return c;}
internal Vec2F32 div_2f32(Vec2F32 a, Vec2F32 b)                 {Vec2F32 c = {a.x/b.x, a.y/b.y}; return c;}
internal Vec2F32 scale_2f32(Vec2F32 v, F32 s)                   {Vec2F32 c = {v.x*s, v.y*s}; return c;}
//internal Vec2F32 clamp_2f32(Vec2F32 a, Vec2F32 x, Vec2F32 b)    {Vec2F32 c = {Clamp(a.x, x.x, b.x), Clamp(a.y, x.y, b.y)}; return c;}
internal F32 min_2f32(Vec2F32 a)                                {F32 c = Min(a.x, a.y); return c;}
internal F32 area_2f32(Vec2F32 a)                               {F32 c = a.x *a.y; return c;}
internal F32 dot_2f32(Vec2F32 a, Vec2F32 b)                     {F32 c = a.x*b.x + a.y*b.y; return c;}
internal F32 length_squared_2f32(Vec2F32 v)                     {F32 c = v.x*v.x + v.y*v.y; return c;}
internal F32 length_2f32(Vec2F32 v)                             {F32 c = sqrt_f32(v.x*v.x + v.y*v.y); return c;}
internal Vec2F32 normalize_2f32(Vec2F32 v)                      {v = scale_2f32(v, 1.f/length_2f32(v)); return v;}


internal Vec3F32 vec_3f32(F32 x, F32 y, F32 z)                  {Vec3F32 v = {x, y, z}; return v;}
internal Vec3F32 add_3f32(Vec3F32 a, Vec3F32 b)                 {Vec3F32 c = {a.x+b.x, a.y+b.y, a.z+b.z}; return c;}
internal Vec3F32 sub_3f32(Vec3F32 a, Vec3F32 b)                 {Vec3F32 c = {a.x-b.x, a.y-b.y, a.z-b.z}; return c;}
internal Vec3F32 mul_3f32(Vec3F32 a, Vec3F32 b)                 {Vec3F32 c = {a.x*b.x, a.y*b.y, a.z*b.z}; return c;}
internal Vec3F32 div_3f32(Vec3F32 a, Vec3F32 b)                 {Vec3F32 c = {a.x/b.x, a.y/b.y, a.z/b.z}; return c;}
internal Vec3F32 scale_3f32(Vec3F32 v, F32 s)                   {Vec3F32 c = {v.x*s, v.y*s, v.z*s}; return c;}
internal F32 dot_3f32(Vec3F32 a, Vec3F32 b)                     {F32 c = a.x*b.x + a.y*b.y + a.z*b.z; return c;}
internal F32 length_squared_3f32(Vec3F32 v)                     {F32 c = v.x*v.x + v.y*v.y + v.z*v.z; return c;}
internal F32 length_3f32(Vec3F32 v)                             {F32 c = sqrt_f32(v.x*v.x + v.y*v.y + v.z*v.z); return c;}
internal Vec3F32 normalize_3f32(Vec3F32 v)                      {v = scale_3f32(v, 1.f/length_3f32(v)); return v;}
internal Vec3F32 cross_3f32(Vec3F32 a, Vec3F32 b)               {Vec3F32 c = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; return c;}

internal Vec3S32 vec_3s32(S32 x, S32 y, S32 z)                  {Vec3S32 v = {x, y, z}; return v;}
internal Vec3S32 add_3s32(Vec3S32 a, Vec3S32 b)                 {Vec3S32 c = {a.x+b.x, a.y+b.y, a.z+b.z}; return c;}
internal Vec3S32 sub_3s32(Vec3S32 a, Vec3S32 b)                 {Vec3S32 c = {a.x-b.x, a.y-b.y, a.z-b.z}; return c;}
internal Vec3S32 mul_3s32(Vec3S32 a, Vec3S32 b)                 {Vec3S32 c = {a.x*b.x, a.y*b.y, a.z*b.z}; return c;}
internal Vec3S32 div_3s32(Vec3S32 a, Vec3S32 b)                 {Vec3S32 c = {a.x/b.x, a.y/b.y, a.z/b.z}; return c;}
internal Vec3S32 scale_3s32(Vec3S32 v, S32 s)                   {Vec3S32 c = {v.x*s, v.y*s, v.z*s}; return c;}
internal S32 dot_3s32(Vec3S32 a, Vec3S32 b)                     {S32 c = a.x*b.x + a.y*b.y + a.z*b.z; return c;}
internal S32 length_squared_3s32(Vec3S32 v)                     {S32 c = v.x*v.x + v.y*v.y + v.z*v.z; return c;}
internal S32 length_3s32(Vec3S32 v)                             {S32 c = (S32)sqrt_f32((F32)(v.x*v.x + v.y*v.y + v.z*v.z)); return c;}
internal Vec3S32 normalize_3s32(Vec3S32 v)                      {v = scale_3s32(v, (S32)(1.f/length_3s32(v))); return v;}
internal Vec3S32 cross_3s32(Vec3S32 a, Vec3S32 b)               {Vec3S32 c = {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; return c;}

internal Vec4F32 vec_4f32(F32 x, F32 y, F32 z, F32 w)           {Vec4F32 v = {x, y, z, w}; return v;}
internal Vec4F32 add_4f32(Vec4F32 a, Vec4F32 b)                 {Vec4F32 c = {a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w}; return c;}
internal Vec4F32 sub_4f32(Vec4F32 a, Vec4F32 b)                 {Vec4F32 c = {a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w}; return c;}
internal Vec4F32 mul_4f32(Vec4F32 a, Vec4F32 b)                 {Vec4F32 c = {a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w}; return c;}
internal Vec4F32 div_4f32(Vec4F32 a, Vec4F32 b)                 {Vec4F32 c = {a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w}; return c;}
internal Vec4F32 scale_4f32(Vec4F32 v, F32 s)                   {Vec4F32 c = {v.x*s, v.y*s, v.z*s, v.w*s}; return c;}
internal F32 dot_4f32(Vec4F32 a, Vec4F32 b)                     {F32 c = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; return c;}
internal F32 length_squared_4f32(Vec4F32 v)                     {F32 c = v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w; return c;}
internal F32 length_4f32(Vec4F32 v)                             {F32 c = sqrt_f32(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); return c;}
internal Vec4F32 normalize_4f32(Vec4F32 v)                      {v = scale_4f32(v, 1.f/length_4f32(v)); return v;}



//~ Range ops

internal Rng1U64 rng_1u64(U64 min, U64 max)                     {Rng1U64 r = {min, max}; if(r.min > r.max) { Swap(U64, r.min, r.max); } return r;}
internal Rng1U64 shift_1u64(Rng1U64 r, U64 x)                   {r.min += x; r.max += x; return r;}
internal Rng1U64 pad_1u64(Rng1U64 r, U64 x)                     {r.min -= x; r.max += x; return r;}
internal U64 center_1u64(Rng1U64 r)                             {U64 c = (r.min+r.max)/2; return c;}
internal B32 contains_1u64(Rng1U64 r, U64 x)                    {B32 c = (r.min <= x && x < r.max); return c;}
internal b32 contains_1i32(Rng1i32 r, i32 x)    {b32 c = (r.min <= x && x < r.max); return c;}
internal U64 dim_1u64(Rng1U64 r)                                {U64 c = ((r.max > r.min) ? (r.max - r.min) : 0); return c;}
internal Rng1U64 union_1u64(Rng1U64 a, Rng1U64 b)               {Rng1U64 c = {Min(a.min, b.min), Max(a.max, b.max)}; return c;}
internal Rng1U64 intersect_1u64(Rng1U64 a, Rng1U64 b)           {Rng1U64 c = {Max(a.min, b.min), Min(a.max, b.max)}; return c;}
internal U64 clamp_1u64(Rng1U64 r, U64 v)                       {v = Clamp(r.min, v, r.max); return v;}

internal Rng2F32 rng_2f32(Vec2F32 min, Vec2F32 max)             {Rng2F32 r = {min, max}; return r;}
internal Rng2F32 shift_2f32(Rng2F32 r, Vec2F32 x)               {r.min = add_2f32(r.min, x); r.max = add_2f32(r.max, x); return r;}
internal Rng2F32 pad_2f32(Rng2F32 r, F32 x)                     {Vec2F32 xv = {x, x}; r.min = sub_2f32(r.min, xv); r.max = add_2f32(r.max, xv); return r;}
internal Vec2F32 center_2f32(Rng2F32 r)                         {Vec2F32 c = {(r.min.x+r.max.x)/2, (r.min.y+r.max.y)/2}; return c;}
internal B32 contains_2f32(Rng2F32 r, Vec2F32 x)                {B32 c = (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y); return c;}
internal Vec2F32 dim_2f32(Rng2F32 r)                            {Vec2F32 dim = {((r.max.x > r.min.x) ? (r.max.x - r.min.x) : 0), ((r.max.y > r.min.y) ? (r.max.y - r.min.y) : 0)}; return dim;}
internal Rng2F32 union_2f32(Rng2F32 a, Rng2F32 b)               {Rng2F32 c; c.p0.x = Min(a.min.x, b.min.x); c.p0.y = Min(a.min.y, b.min.y); c.p1.x = Max(a.max.x, b.max.x); c.p1.y = Max(a.max.y, b.max.y); return c;}
internal Rng2F32 intersect_2f32(Rng2F32 a, Rng2F32 b)           {Rng2F32 c; c.p0.x = Max(a.min.x, b.min.x); c.p0.y = Max(a.min.y, b.min.y); c.p1.x = Min(a.max.x, b.max.x); c.p1.y = Min(a.max.y, b.max.y); return c;}
internal Vec2F32 clamp_2f32(Rng2F32 r, Vec2F32 v)               {v.x = Clamp(r.min.x, v.x, r.max.x); v.y = Clamp(r.min.y, v.y, r.max.y); return v;}

internal Rng2S32 rng_2s32(Vec2S32 min, Vec2S32 max)             {Rng2S32 r = {min, max}; return r;}
internal Rng2S32 shift_2s32(Rng2S32 r, Vec2S32 x)               {r.min = add_2s32(r.min, x); r.max = add_2s32(r.max, x); return r;}
internal Rng2S32 pad_2s32(Rng2S32 r, S32 x)                     {Vec2S32 xv = {x, x}; r.min = sub_2s32(r.min, xv); r.max = add_2s32(r.max, xv); return r;}
internal Vec2S32 center_2s32(Rng2S32 r)                         {Vec2S32 c = {(r.min.x+r.max.x)/2, (r.min.y+r.max.y)/2}; return c;}
internal B32 contains_2s32(Rng2S32 r, Vec2S32 x)                {B32 c = (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y); return c;}
internal Vec2S32 dim_2s32(Rng2S32 r)                            {Vec2S32 dim = {((r.max.x > r.min.x) ? (r.max.x - r.min.x) : 0), ((r.max.y > r.min.y) ? (r.max.y - r.min.y) : 0)}; return dim;}
internal Rng2S32 union_2s32(Rng2S32 a, Rng2S32 b)               {Rng2S32 c; c.p0.x = Min(a.min.x, b.min.x); c.p0.y = Min(a.min.y, b.min.y); c.p1.x = Max(a.max.x, b.max.x); c.p1.y = Max(a.max.y, b.max.y); return c;}
internal Rng2S32 intersect_2s32(Rng2S32 a, Rng2S32 b)           {Rng2S32 c; c.p0.x = Max(a.min.x, b.min.x); c.p0.y = Max(a.min.y, b.min.y); c.p1.x = Min(a.max.x, b.max.x); c.p1.y = Min(a.max.y, b.max.y); return c;}
internal Vec2S32 clamp_2s32(Rng2S32 r, Vec2S32 v)               {v.x = Clamp(r.min.x, v.x, r.max.x); v.y = Clamp(r.min.y, v.y, r.max.y); return v;}

internal Mat3x3F32
mat_3x3f32(F32 diagonal)
{
	Mat3x3F32 result = {0};
	result.v[0][0] = diagonal;
	result.v[1][1] = diagonal;
	result.v[2][2] = diagonal;
	return result;
}

internal Mat3x3F32
make_translate_3x3f32(Vec2F32 delta)
{
	Mat3x3F32 mat = mat_3x3f32(1.f);
	mat.v[0][2] = delta.x;
	mat.v[1][2] = delta.y;
	return mat;
}

internal Mat3x3F32
make_scale_3x3f32(Vec2F32 scale)
{
	Mat3x3F32 mat = mat_3x3f32(1.f);
	mat.v[0][0] = scale.x;
	mat.v[1][1] = scale.y;
	return mat;
}

internal Mat3x3F32
mul_3x3f32(Mat3x3F32 a, Mat3x3F32 b)
{
	Mat3x3F32 c = {0};
	for(int j = 0; j < 3; j += 1)
	{
		for(int i = 0; i < 3; i += 1)
		{
			c.v[i][j] = (a.v[0][j]*b.v[i][0] +
						 a.v[1][j]*b.v[i][1] +
						 a.v[2][j]*b.v[i][2]);
		}
	}
	return c;
}

internal Mat3x3F32
inverse_3x3f32(Mat3x3F32 a)
{
	Mat3x3F32 r = {0};
	r32 det = 
		a.v[0][0] * (a.v[1][1] * a.v[2][2] - a.v[1][2] * a.v[2][1]) -
		a.v[0][1] * (a.v[1][0] * a.v[2][2] - a.v[1][2] * a.v[2][0]) +
		a.v[0][2] * (a.v[1][0] * a.v[2][1] - a.v[1][1] * a.v[2][0]);
	if(abs_f32(det) > 0.00001f)
	{
		r32 inv_det = 1.f / det;
		r.v[0][0] = (a.v[1][1] * a.v[2][2] - a.v[2][1] * a.v[1][2]) * inv_det;
		r.v[0][1] = (a.v[0][2] * a.v[2][1] - a.v[0][1] * a.v[2][2]) * inv_det;
		r.v[0][2] = (a.v[0][1] * a.v[1][2] - a.v[0][2] * a.v[1][1]) * inv_det;
		r.v[1][0] = (a.v[1][2] * a.v[2][0] - a.v[1][0] * a.v[2][2]) * inv_det;
		r.v[1][1] = (a.v[0][0] * a.v[2][2] - a.v[0][2] * a.v[2][0]) * inv_det;
		r.v[1][2] = (a.v[1][0] * a.v[0][2] - a.v[0][0] * a.v[1][2]) * inv_det;
		r.v[2][0] = (a.v[1][0] * a.v[2][1] - a.v[2][0] * a.v[1][1]) * inv_det;
		r.v[2][1] = (a.v[2][0] * a.v[0][1] - a.v[0][0] * a.v[2][1]) * inv_det;
		r.v[2][2] = (a.v[0][0] * a.v[1][1] - a.v[1][0] * a.v[0][1]) * inv_det;
	}
	return r;
}

internal Vec2F32
transform_3x3f32(Mat3x3F32 a, Vec2F32 v)
{
	Vec2F32 r = {0};
	r.x = a.v[0][0]*v.x + a.v[0][1]*v.y + a.v[0][2];
	r.y = a.v[1][0]*v.x + a.v[1][1]*v.y + a.v[1][2];
	return r;
}

internal r32
lerp(r32 a, r32 f, r32 b)
{
	r32 result = a + f * (b - a);
	return result;
}

inline internal i32 
round_r32_to_i32(r32 val)
{
    i32 result = (i32) (val + 0.5f);
    return result;
}

inline internal i32 
sign_of(i32 val)
{
    i8 result = (val >= 0) ? 1 : -1;
    return result;
}

inline internal b32 
operator==(V2i a, V2i b)
{
    b32 result = (a.x == b.x) && (a.y == b.y);
    return result;
}

inline internal b32 
operator<(V2i a, V2i b)
{
    b32 result = (a.x < b.x) && (a.y < b.y);
    return result;
}

inline internal b32 
operator>(V2i a, V2i b)
{
    b32 result = (a.x > b.x) && (a.y > b.y);
    return result;
}

inline internal b32 
operator!=(V2i a, V2i b)
{
    b32 result = !(a == b);
    return result;
}

inline internal V2i 
operator-(V2i a, V2i b)
{
    V2i result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline internal V2i 
operator-(V2i a)
{
    V2i result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

inline internal void 
operator-=(V2i &a, V2i b)
{
    a.x -= b.x;
    a.y -= b.y;
}

inline internal V2i 
operator+(V2i a, V2i b)
{
    V2i result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline internal void 
operator+=(V2i &a, V2i b)
{
    a.x += b.x;
    a.y += b.y;
}

inline internal V2i 
operator*(i32 a, V2i b)
{
    V2i result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

inline internal V2i 
operator*(V2i a, V2i b)
{
    V2i result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}

inline internal V2i 
operator/(V2i a, V2i b)
{
    V2i result;
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    return result;
}

inline internal V2i 
operator/(V2i a, i32 b)
{
    V2i result;
    result.x = a.x / b;
    result.y = a.y / b;
    return result;
}

inline internal Line 
init_conversion(r32 slope, r32 offset = 0.0f)
{
    Line line = {0};
    line.slope = slope;
    line.offset = offset;
    return line;
}

internal U32
u32_from_rgba(Vec4F32 rgba)
{
	U32 result = 0;
	result |= ((U32)((U8)(rgba.x*255.f))) << 24;
	result |= ((U32)((U8)(rgba.y*255.f))) << 16;
	result |= ((U32)((U8)(rgba.z*255.f))) <<  8;
	result |= ((U32)((U8)(rgba.w*255.f))) <<  0;
	return result;
}

internal Vec4F32
rgba_from_u32(U32 hex)
{
	Vec4F32 result = v4f32(((hex&0xff000000)>>24)/255.f,
						   ((hex&0x00ff0000)>>16)/255.f,
						   ((hex&0x0000ff00)>> 8)/255.f,
						   ((hex&0x000000ff)>> 0)/255.f);
	return result;
}
