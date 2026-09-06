#ifndef BASE_STRINGS_H
#define BASE_STRINGS_H

struct String8
{
	u8 *str;
	u64 size;
};

struct String16
{
	U16 *str;
	U64 size;
};

struct String32
{
	U32 *str;
	U64 size;
};

struct String8Node
{
	String8Node *next;
	String8 string;
};

struct String8List
{
	String8Node *first;
	String8Node *last;
	u64 node_count;
	u64 total_size;
};

struct String8Array
{
	String8 *v;
	u64 count;
};

struct CharArray
{
	char **v;
	u64 count;
};

typedef u32 StringMatchFlags;
enum
{
	StringMatchFlag_CaseInsensitive  = (1 << 0),
	StringMatchFlag_SlashInsensitive = (1 << 1),
};

struct StringJoin
{
	String8 pre;
	String8 sep;
	String8 post;
};

struct UnicodeDecode
{
	U32 inc;
	U32 codepoint;
};

//
//~ rsb: Constructors
//

#define str8_lit(s) str8((u8 *)(s), sizeof(s) - 1)
#define str8_lit_comp(s) {(u8 *)(s), sizeof(s) - 1}
#define str8_varg(s) (int)((s).size), ((s).str)

#define str8_array(S,C) str8((U8*)(S), sizeof(*(S))*(C))
#define str8_array_fixed(S) str8((U8*)(S), sizeof(S))
#define str8_struct(S) str8((U8*)(S), sizeof(*(S)))

internal u64 cstring8_length(u8 *c);
internal String8  str8(U8 *str, U64 size);
internal String8  str8_range(U8 *first, U8 *one_past_last);
internal String8  str8_zero(void);
internal String16 str16(U16 *str, U64 size);
internal String16 str16_range(U16 *first, U16 *one_past_last);
internal String16 str16_zero(void);
internal String32 str32(U32 *str, U64 size);
internal String32 str32_range(U32 *first, U32 *one_past_last);
internal String32 str32_zero(void);
internal String8  str8_cstring(char *c);
internal String16 str16_cstring(U16 *c);
internal String32 str32_cstring(U32 *c);

//
//~ rsb: Char conversions
//

internal b32 char_is_space(u8 c);
internal b32 char_is_lower(u8 c);
internal b32 char_is_upper(u8 c);
internal b32 char_is_alpha(u8 c);
internal b32 char_is_slash(u8 c);
internal u8 char_to_lower(u8 c);
internal u8 char_to_upper(u8 c);
internal u8 char_to_correct_slash(u8 c);

//
//~ rsb: String slicing
//

internal String8 str8_prefix(String8 str, u64 size);
internal String8 str8_postfix(String8 str, u64 size);
internal String8 str8_substr(String8 str, Rng1u64 range);
internal String8 str8_skip(String8 str, u64 amt);
internal String8 str8_chop(String8 str, u64 amt);

//
//~ rsb: Substrings
//

internal b32 str8_match(String8 s1, String8 s2, StringMatchFlags flags);
internal u64 str8_find_needle(String8 string, u64 start_pos, String8 needle, StringMatchFlags flag);
internal u64 str8_find_needle_reverse(String8 string, u64 start_pos, String8 needle, StringMatchFlags flags);
internal b32 str8_ends_with(String8 string, String8 end, StringMatchFlags flags);
internal b32 str8_replace_substr(String8 master, u64 pos, String8 replace);
internal String8List str8_split(Arena *arena, String8 str, u8 *split_chars, u64 split_char_count);
internal String8List str8_split_path(Arena *arena, String8 path);

//
//~ rsb: Allocators
//

internal String8 push_str8_cat(Arena *arena, String8 s1, String8 s2);
internal String8 push_str8f(Arena *arena, char *fmt, ...);
internal String8 push_str8fv(Arena *arena, char *fmt, va_list args);
internal String8 push_str8_copy(Arena *arena, String8 string);

//
//~ rsb: String list
//

internal String8Node *str8_list_push_node(String8List *list, String8Node *n);
internal String8Node *str8_list_push_node_front(String8List *list, String8Node *n);
internal String8Node * str8_list_push(Arena *arena, String8List *list, String8 string);
internal String8 str8_list_join(Arena *arena, String8List *list, StringJoin *join);
internal String8 str8_list_join_path(Arena *arena, String8List *list);
internal String8 str8_chop_last_slash(String8 string);

//
//~ rsb: Serialization
//

internal void str8_serial_begin(Arena *arena, String8List *srl);
internal String8 str8_serial_end(Arena *arena, String8List *srl);
internal void *str8_serial_push_size(Arena *arena, String8List *srl ,u64 size);
internal void *str8_serial_push_data(Arena *arena, String8List *srl, void *data, u64 size);
internal void str8_serial_push_u64(Arena *arena, String8List *srl, u64 x);
internal void str8_serial_push_u32(Arena *arena, String8List *srl, u32 x);
internal void str8_serial_push_u8(Arena *arena, String8List *srl, u8 x);
internal void str8_serial_push_string(Arena *arena, String8List *srl, String8 str);
internal void str8_serial_push_cstr(Arena *arena, String8List *srl, String8 str);
#define str8_serial_push_array(arena, srl, ptr, count) str8_serial_push_data(arena, srl, ptr, (count) * sizeof(*(ptr)))
#define str8_serial_push_struct(arena, srl, ptr) str8_serial_push_array(arena, srl, ptr, 1)

//
//~ rsb: Deserialization
//

internal u64 str8_deserial_read(String8 string, u64 off, void *read_dst, u64 read_size, u64 granularity);
internal u64 str8_deserial_read_block(String8 string, u64 off, u64 size, String8 *block_out);
#define str8_deserial_read_array(string, off, ptr, count) str8_deserial_read((string), (off), (ptr), sizeof(*(ptr))*(count), sizeof(*(ptr)))
#define str8_deserial_read_struct(string, off, ptr)       str8_deserial_read_array(string, off, ptr, 1)

//
//~ rjf: UTF-8 & UTF-16 Decoding/Encoding
//

internal UnicodeDecode utf8_decode(U8 *str, U64 max);
internal UnicodeDecode utf16_decode(U16 *str, U64 max);
internal U32 utf8_encode(U8 *str, U32 codepoint);
internal U32 utf16_encode(U16 *str, U32 codepoint);
internal U32 utf8_from_utf32_single(U8 *buffer, U32 character);

//
//~ rjf: Unicode String Conversions
//

internal String8 str8_from_16(Arena *arena, String16 in);
internal String16 str16_from_8(Arena *arena, String8 in);
internal String8 str8_from_32(Arena *arena, String32 in);
internal String32 str32_from_8(Arena *arena, String8 in);

#endif //BASE_STRINGS_H
