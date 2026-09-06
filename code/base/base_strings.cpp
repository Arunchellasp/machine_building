
//
//~ rsb: Constructors
//

internal u64 
cstring8_length(u8 *c)
{
    if (!c) return 0;
	u8 *p = c;
	for(; *p != 0; p += 1);
	return (p - c);
}

internal U64
cstring16_length(U16 *c)
{
	U16 *p = c;
	for (;*p != 0; p += 1);
	return(p - c);
}

internal String8
str8(u8 *str, u64 size)
{
	String8 result;
	result.str = str;
	result.size = size;
	return result;
}

internal String8
str8_zero(void){
	String8 result = {0};
	return(result);
}

internal String16
str16(U16 *str, U64 size){
	String16 result = {str, size};
	return(result);
}

internal String16
str16_range(U16 *first, U16 *one_past_last){
	String16 result = {first, (U64)(one_past_last - first)};
	return(result);
}

internal String16
str16_zero(void){
	String16 result = {0};
	return(result);
}

internal String32
str32(U32 *str, U64 size){
	String32 result = {str, size};
	return(result);
}

internal String32
str32_range(U32 *first, U32 *one_past_last){
	String32 result = {first, (U64)(one_past_last - first)};
	return(result);
}

internal String32
str32_zero(void){
	String32 result = {0};
	return(result);
}

internal String8
str8_cstring(char *str)
{
	return str8((u8 *)str, cstring8_length((u8 *)str));
}

internal String16
str16_cstring(U16 *c)
{
	String16 result = {(U16*)c, cstring16_length((U16*)c)};
	return(result);
}

internal String8
str8_range(u8 *first, u8 *one_past_last)
{
	String8 result = {first, (u64)(one_past_last - first)}; 
	return result;
}

internal String8
str8_cstring_capped(void *cstr, void *cap)
{
	char *ptr = (char *)cstr;
	char *opl = (char *)cap;
	for (;ptr < opl && *ptr != 0; ptr += 1);
	u64 size = (u64)(ptr - (char *)cstr);
	String8 result = str8((u8*)cstr, size);
	return result;
}

//
//~ rsb: Char conversions
//

internal b32
char_is_space(u8 c)
{
	return(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v');
}

internal b32 
char_is_lower(u8 c)
{
	return ('a' <= c && c <= 'z');
}

internal b32 
char_is_upper(u8 c)
{
	return ('A' <= c && c <= 'Z');
}

internal b32
char_is_alpha(u8 c)
{
	return(char_is_upper(c) || char_is_lower(c));
}

internal b32
char_is_slash(u8 c)
{
	return (c == '\\' || c == '/');
}

internal u8
char_to_lower(u8 c)
{
	return (char_is_upper(c) ? (c - 'A' + 'a') : c);
}

internal u8
char_to_upper(u8 c)
{
	return (char_is_lower(c) ? (c - 'a' + 'A') : c);
}

internal u8
char_to_correct_slash(u8 c)
{
	return (char_is_slash(c) ? '/' : c);
}

//
//~ rsb: String slicing
//

internal String8
str8_prefix(String8 str, u64 size)
{
	str.size = ClampTop(size, str.size);
	return str;
}

internal String8
str8_postfix(String8 str, u64 size)
{
	size = ClampTop(size, str.size);
	str.str += (str.size - size);
	str.size = size;
	return str;
}

internal String8
str8_substr(String8 str, Rng1U64 range)
{
	u64 min = ClampTop(range.min, str.size);
	str.str += min;
	str.size = dim_1u64(range);
	return str;
}

internal String8
str8_skip(String8 str, u64 amt)
{
	amt = ClampTop(amt, str.size);
	str.str += amt;
	str.size -= amt;
	return str;
}

internal String8
str8_chop(String8 str, u64 amt)
{
	amt = ClampTop(amt, str.size);
	str.size -= amt;
	return str;
}

//- rjf: string to int

// NOTE(allen): Includes reverses for uppercase and lowercase hex.
read_only global U8 integer_symbol_reverse[128] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

internal s64
sign_from_str8(String8 string, String8 *string_tail){
	// count negative signs
	u64 neg_count = 0;
	u64 i = 0;
	for (; i < string.size; i += 1){
		if (string.str[i] == '-'){
			neg_count += 1;
		}
		else if (string.str[i] != '+'){
			break;
		}
	}
	
	// output part of string after signs
	*string_tail = str8_skip(string, i);
	
	// output integer sign
	s64 sign = (neg_count & 1)?-1:+1;
	return(sign);
}

internal B32
str8_is_integer(String8 string, U32 radix){
	B32 result = 0;
	if (string.size > 0){
		if (1 < radix && radix <= 16){
			result = 1;
			for (u64 i = 0; i < string.size; i += 1){
				U8 c = string.str[i];
				if (!(c < 0x80) || integer_symbol_reverse[c] >= radix){
					result = 0;
					break;
				}
			}
		}
	}
	return(result);
}

internal u64
u64_from_str8(String8 string, U32 radix){
	u64 x = 0;
	if (1 < radix && radix <= 16){
		for (u64 i = 0; i < string.size; i += 1){
			x *= radix;
			x += integer_symbol_reverse[string.str[i]&0x7F];
		}
	}
	return(x);
}

internal s64
s64_from_str8(String8 string, U32 radix){
	s64 sign = sign_from_str8(string, &string);
	s64 x = (s64)u64_from_str8(string, radix) * sign;
	return(x);
}

//
//~ rsb: Substrings
//

//- rsb: Empty strings match by default
internal b32
str8_match(String8 s1, String8 s2, StringMatchFlags flags)
{
	b32 match = 0;
	if(s1.size == s2.size)
	{
		b32 case_insensitive  = flags & StringMatchFlag_CaseInsensitive;
		b32 slash_insensitive = flags & StringMatchFlag_SlashInsensitive;
		u64 size              = Min(s1.size, s2.size);
		match = 1; 
		for(u64 it = 0; it < size; it += 1)
		{
			u8 at = s1.str[it];
			u8 bt = s2.str[it];
			if(case_insensitive)
			{
				at = char_to_lower(at);
				bt = char_to_lower(bt);
			}
			if(slash_insensitive)
			{
				at = char_to_correct_slash(at);
				bt = char_to_correct_slash(bt);
			}
			if(at != bt)
			{
				match = 0;
				break;
			}
		}
	}
	return match;
}

internal u64
str8_find_needle(String8 string, u64 start_pos, String8 needle, StringMatchFlags flags)
{
	u64 result = string.size;
	for(u64 pos = start_pos; 
		pos <= (Max(string.size, needle.size) - needle.size); 
		pos += 1)
	{
		String8 haystack = str8_substr(string, rng_1u64(pos, pos + needle.size));
		if(str8_match(haystack, needle, flags))
		{
			result = pos;
			break;
		}
	}
	return result;
}

internal u64
str8_find_needle_reverse(String8 string, u64 start_pos, String8 needle, StringMatchFlags flags)
{
	u64 result = 0;
	for(i64 pos = string.size - start_pos - needle.size; 
		pos >= 0; 
		pos -= 1)
	{
		String8 haystack = str8_substr(string, rng_1u64(pos, pos + needle.size));
		if(str8_match(haystack, needle, flags))
		{
			result = (u64)pos + needle.size;
			break;
		}
	}
	return result;
}

internal b32
str8_ends_with(String8 string, String8 end, StringMatchFlags flags)
{
	String8 postfix = str8_postfix(string, end.size);
	b32 result = str8_match(postfix, end, flags);
	return result;
}

internal b32
str8_replace_substr(String8 master, u64 pos, String8 replace)
{
	u64 offset = ClampTop(pos, master.size);
	u64 size = ClampTop(offset + replace.size, master.size) - offset;
	MemoryCopy(master.str + offset, replace.str, size);
}

internal String8List
str8_split(Arena *arena, String8 str, u8 *split_chars, u64 split_char_count)
{
	String8List list = {0};
	u8 *ptr = str.str;
	u8 *end = ptr + str.size;
	for(; ptr < end; )
	{
		u8 *first = ptr;
		for(; ptr < end; ptr += 1)
		{
			b32 is_split = 0;
			u8 c = *ptr;
			for(u64 it = 0; it < split_char_count; it += 1)
			{
				if(split_chars[it] == c)
				{
					is_split = 1;
					break;
				}
			}
			if(is_split)
			{
				break;
			}
		}
		
		String8 sub_str = str8_range(first, ptr);
		if(sub_str.size)
		{
			str8_list_push(arena, &list, sub_str);
		}
		ptr += 1;
	}
	return list;
}

internal String8List
str8_split_path(Arena *arena, String8 path)
{
	String8List result = str8_split(arena, path, (u8 *)"/\\", 2);
	return result;
}

//
//~ rsb: Allocators
//

internal String8
push_str8_cat(Arena *arena, String8 s1, String8 s2)
{
	String8 str;
	str.size = s1.size + s2.size;
	str.str = push_array_no_zero(arena, u8, str.size + 1);
	MemoryCopy(str.str, s1.str, s1.size);
	MemoryCopy(str.str + s1.size, s2.str, s2.size);
	str.str[str.size] = 0;
	return str;
}

internal String8
push_str8fv(Arena *arena, char *fmt, va_list args)
{
	va_list args2;
	va_copy(args2, args);
	u64 needed_bytes = stbsp_vsnprintf(0, 0, fmt, args) + 1;
	String8 result = {0};
	result.str = push_array(arena, u8, needed_bytes);
	result.size = stbsp_vsnprintf((char *)result.str, needed_bytes, fmt, args2);;
	result.str[result.size] = 0;
	va_end(args2);
	return result;
}

internal String8
push_str8f(Arena *arena, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	String8 result = push_str8fv(arena, fmt, args);
	va_end(args);
	return result;
}

internal String8
push_str8_copy(Arena *arena, String8 string)
{
	String8 result;
	result.str = push_array(arena, u8, string.size + 1);
	result.size = string.size;
	MemoryCopy(result.str, string.str, string.size);
	result.str[result.size] = 0;
	return result;
}

//
//~ rsb: String list
//

internal String8Node *
str8_list_push_node(String8List *list, String8Node *n)
{
	SLLQueuePush(list->first, list->last, n);
	list->node_count += 1;
	list->total_size += n->string.size;
	return n;
}

internal String8Node *
str8_list_push_node_front(String8List *list, String8Node *n)
{
	SLLQueuePushFront(list->first, list->last, n);
	list->node_count += 1;
	list->total_size += n->string.size;
	return n;
}

internal String8Node *
str8_list_push_front(Arena *arena, String8List *list, String8 string)
{
	String8Node *node = push_array_no_zero(arena, String8Node, 1);
	node->string = string;
	str8_list_push_node_front(list, node);
	return node;
}

internal String8Node *
str8_list_push(Arena *arena, String8List *list, String8 string)
{
	String8Node *node = push_array_no_zero(arena, String8Node, 1);
	node->string = string;
	str8_list_push_node(list, node);
	return node;
}

internal String8Node*
str8_list_pushf(Arena *arena, String8List *list, char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	String8 string = push_str8fv(arena, fmt, args);
	String8Node *result = str8_list_push(arena, list, string);
	va_end(args);
	return(result);
}

internal void
str8_list_concat_in_place(String8List *list, String8List *to_push)
{
	if(to_push->node_count != 0)
	{
		if(list->last)
		{
			list->node_count += to_push->node_count;
			list->total_size += to_push->total_size;
			list->last->next = to_push->first;
			list->last = to_push->last;
		}
		else
		{
			*list = *to_push;
		}
		MemoryZeroStruct(to_push);
	}
}

internal String8Array
str8_array_from_list(Arena *arena, String8List *list)
{
	String8Array array = {0};
	array.v = push_array_no_zero(arena, String8, list->node_count);
	array.count = list->node_count;
	u64 idx = 0;
	for(String8Node *n = list->first; n != 0; n = n->next, idx += 1)
	{
		array.v[idx] = n->string;
	}
	return array;
}

internal String8Array
str8_array_from_list_copy(Arena *arena, String8List *list)
{
	String8Array array = {0};
	array.v = push_array_no_zero(arena, String8, list->node_count);
	array.count = list->node_count;
	u64 idx = 0;
	for(String8Node *n = list->first; n != 0; n = n->next, idx += 1)
	{
		array.v[idx] = push_str8_copy(arena, n->string);
	}
	return array;
}

internal String8
str8_list_join(Arena *arena, String8List *list, StringJoin *join)
{
	u64 sep_count = 0;
	if(list->node_count)
	{
		sep_count = list->node_count - 1;
	}
	
	String8 result;
	result.size = list->total_size + join->pre.size + sep_count * join->sep.size + join->post.size;
	u8 *ptr = result.str = push_array_no_zero(arena, u8, result.size + 1);
	MemoryCopy(ptr, join->pre.str, join->pre.size);
	ptr += join->pre.size;
	for(String8Node *node = list->first; node != 0; node = node->next)
	{
		MemoryCopy(ptr, node->string.str, node->string.size);
		ptr += node->string.size;
		if(node->next != 0)
		{
			MemoryCopy(ptr, join->sep.str, join->sep.size);
			ptr += join->sep.size;
		}
	}
	MemoryCopy(ptr, join->post.str, join->post.size);
	ptr += join->post.size;
	*ptr = 0;
	return result;
}

internal String8Array
str8array_from_chararray(Arena *arena, CharArray array)
{
	String8Array result = {0};
	if(array.count)
	{
		result.count = array.count;
		result.v = push_array(arena, String8, array.count);
		for(u64 it = 0; it < array.count; it++)
		{
			result.v[it] = str8_cstring(array.v[it]);
		}
	}
	return result;
}

internal CharArray
chararray_from_str8array(Arena *arena, String8Array array)
{
	CharArray result = {0};
	if(array.count)
	{
		result.count = array.count;
		result.v = push_array(arena, char *, array.count);
		for(u64 it = 0; it < array.count; it++)
		{
			result.v[it] = (char *)array.v[it].str;
		}
	}
	return result;
}

internal String8Array
str8_array_copy(Arena *arena, String8Array src)
{
	String8Array result = {0};
	result.count = src.count;
	result.v = push_array(arena, String8, src.count);
	for(u64 it = 0; it < src.count; it += 1)
	{
		result.v[it] = push_str8_copy(arena, src.v[it]);
	}
	return result;
}

//
//~ rsb: Path helpers
//

internal String8
str8_list_join_path(Arena *arena, String8List *list)
{
	StringJoin join = {.sep = str8_lit("/")};
	String8 result = str8_list_join(arena, list, &join);
	return result;
}

internal String8
str8_chop_last_slash(String8 string)
{
	u8 *ptr = string.str + string.size - 1;
    for(;ptr >= string.str; ptr -= 1)
	{
		if(*ptr == '/' || *ptr == '\\')
		{
			break;
		}
    }
    if(ptr >= string.str)
	{
		string.size = (u64)(ptr - string.str);
    }
    else
	{
		string.size = 0;
    }
	return(string);
}

internal String8
str8_skip_last_slash(String8 string)
{
	if (string.size > 0)
	{
		u8 *ptr = string.str + string.size - 1;
		for(;ptr >= string.str; ptr -= 1)
		{
			if(*ptr == '/' || *ptr == '\\')
			{
				break;
			}
		}
		if(ptr >= string.str)
		{
			ptr += 1;
			string.size = (u64)(string.str + string.size - ptr);
			string.str = ptr;
		}
	}
	return(string);
}

internal String8
str8_chop_last_dot(String8 string)
{
	String8 result = string;
	U64 p = string.size;
	for(;p > 0;)
	{
		p -= 1;
		if(string.str[p] == '.')
		{
			result = str8_prefix(string, p);
			break;
		}
	}
	return result;
}

internal String8
str8_skip_last_dot(String8 string)
{
	String8 result = string;
	U64 p = string.size;
	for(;p > 0;)
	{
		p -= 1;
		if(string.str[p] == '.')
		{
			result = str8_skip(string, p + 1);
			break;
		}
	}
	return result;
}

//
//~ rsb: Serialization 
//

internal void
str8_serial_begin(Arena *arena, String8List *srl)
{
	MemoryZeroStruct(srl);
	String8Node *node = push_array(arena, String8Node, 1);
	node->string.str = push_array(arena, u8, 0);
	str8_list_push_node(srl, node);
}

internal String8
str8_serial_end(Arena *arena, String8List *srl)
{
	String8 result = {0};
	result.size = srl->total_size;
	result.str = push_array_no_zero(arena, u8, result.size);
	u8 *ptr = result.str;
	for(String8Node *n = srl->first; n != 0; n = n->next)
	{
		u64 size = n->string.size;
		MemoryCopy(ptr, n->string.str, n->string.size);
		ptr += size;
	}
	return result;
}

internal void *
str8_serial_push_size(Arena *arena, String8List *srl ,u64 size)
{
	void *result = {0};
	if(size != 0)
	{
		u8 *buf = push_array_no_zero_aligned(arena, u8, size, 1);
		String8 *str = &srl->last->string;
		if(str->str + str->size == buf)
		{
			str->size += size;
			srl->total_size += size;
		}
		else
		{
			str8_list_push(arena, srl, str8(buf, size));
		}
		result = buf;
	}
	return result;
}

internal void *
str8_serial_push_data(Arena *arena, String8List *srl, void *data, u64 size)
{
	void *result = str8_serial_push_size(arena, srl, size);
	if(result != 0)
	{
		MemoryCopy(result, data, size);
	}
	return result;
}

internal void
str8_serial_push_u64(Arena *arena, String8List *srl, u64 x)
{
	str8_serial_push_data(arena, srl, &x, sizeof(x));
}

internal void
str8_serial_push_u32(Arena *arena, String8List *srl, u32 x)
{
	str8_serial_push_data(arena, srl, &x, sizeof(x));
}

internal void
str8_serial_push_u8(Arena *arena, String8List *srl, u8 x)
{
	str8_serial_push_data(arena, srl, &x, sizeof(x));
}

internal void
str8_serial_push_cstr(Arena *arena, String8List *srl, String8 str)
{
	str8_serial_push_data(arena, srl, str.str, str.size);
	str8_serial_push_u8(arena, srl, 0);
}

internal void
str8_serial_push_string(Arena *arena, String8List *srl, String8 str)
{
	str8_serial_push_data(arena, srl, str.str, str.size);
}

internal void
str8_serial_push_binary(Arena *arena, String8List *srl ,String8 str)
{
	str8_serial_push_u64(arena, srl, str.size);
	str8_serial_push_data(arena, srl, str.str, str.size);
}

//
//~ rsb: Deserialization 
//

internal u64
str8_deserial_read(String8 string, u64 off, void *read_dst, u64 read_size, u64 granularity)
{
	u64 bytes_left = string.size-Min(off, string.size);
	u64 actually_readable_size = Min(bytes_left, read_size);
	u64 legally_readable_size = actually_readable_size - actually_readable_size%granularity;
	if(legally_readable_size > 0)
	{
		MemoryCopy(read_dst, string.str+off, legally_readable_size);
	}
	return legally_readable_size;
}

internal u64
str8_deserial_read_cstr(String8 string, u64 off, String8 *cstr_out)
{
	u64 cstr_size = 0;
	if(off < string.size)
	{
		U8 *ptr = string.str + off;
		U8 *cap = string.str + string.size;
		*cstr_out = str8_cstring_capped(ptr, cap);
		cstr_size = (cstr_out->size + 1);
	}
	return cstr_size;
}

internal u64
str8_deserial_read_block(String8 string, u64 off, u64 size, String8 *block_out)
{
	Rng1U64 range = rng_1u64(off, off + size);
	*block_out = str8_substr(string, range);
	return block_out->size;
}

internal u64
str8_deserial_read_binary(String8 string, u64 off, String8 *out)
{
	u64 size = {0};
	size += str8_deserial_read_struct(string, off + size, &out->size);
	size += str8_deserial_read_block(string, off + size, out->size, out);
	return size;
}

////////////////////////////////
//~ rjf: UTF-8 & UTF-16 Decoding/Encoding

read_only global U8 utf8_class[32] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

internal UnicodeDecode
utf8_decode(U8 *str, U64 max){
	UnicodeDecode result = {1, max_U32};
	U8 byte = str[0];
	U8 byte_class = utf8_class[byte >> 3];
	switch (byte_class)
	{
		case 1:
		{
			result.codepoint = byte;
		}break;
		case 2:
		{
			if (2 < max)
			{
				U8 cont_byte = str[1];
				if (utf8_class[cont_byte >> 3] == 0)
				{
					result.codepoint = (byte & bitmask5) << 6;
					result.codepoint |=  (cont_byte & bitmask6);
					result.inc = 2;
				}
			}
		}break;
		case 3:
		{
			if (2 < max)
			{
				U8 cont_byte[2] = {str[1], str[2]};
				if (utf8_class[cont_byte[0] >> 3] == 0 &&
					utf8_class[cont_byte[1] >> 3] == 0)
				{
					result.codepoint = (byte & bitmask4) << 12;
					result.codepoint |= ((cont_byte[0] & bitmask6) << 6);
					result.codepoint |=  (cont_byte[1] & bitmask6);
					result.inc = 3;
				}
			}
		}break;
		case 4:
		{
			if (3 < max)
			{
				U8 cont_byte[3] = {str[1], str[2], str[3]};
				if (utf8_class[cont_byte[0] >> 3] == 0 &&
					utf8_class[cont_byte[1] >> 3] == 0 &&
					utf8_class[cont_byte[2] >> 3] == 0)
				{
					result.codepoint = (byte & bitmask3) << 18;
					result.codepoint |= ((cont_byte[0] & bitmask6) << 12);
					result.codepoint |= ((cont_byte[1] & bitmask6) <<  6);
					result.codepoint |=  (cont_byte[2] & bitmask6);
					result.inc = 4;
				}
			}
		}
	}
	return(result);
}

internal UnicodeDecode
utf16_decode(U16 *str, U64 max){
	UnicodeDecode result = {1, max_U32};
	result.codepoint = str[0];
	result.inc = 1;
	if (max > 1 && 0xD800 <= str[0] && str[0] < 0xDC00 && 0xDC00 <= str[1] && str[1] < 0xE000){
		result.codepoint = ((str[0] - 0xD800) << 10) | ((str[1] - 0xDC00) + 0x10000);
		result.inc = 2;
	}
	return(result);
}

internal U32
utf8_encode(U8 *str, U32 codepoint){
	U32 inc = 0;
	if (codepoint <= 0x7F){
		str[0] = (U8)codepoint;
		inc = 1;
	}
	else if (codepoint <= 0x7FF){
		str[0] = (bitmask2 << 6) | ((codepoint >> 6) & bitmask5);
		str[1] = bit8 | (codepoint & bitmask6);
		inc = 2;
	}
	else if (codepoint <= 0xFFFF){
		str[0] = (bitmask3 << 5) | ((codepoint >> 12) & bitmask4);
		str[1] = bit8 | ((codepoint >> 6) & bitmask6);
		str[2] = bit8 | ( codepoint       & bitmask6);
		inc = 3;
	}
	else if (codepoint <= 0x10FFFF){
		str[0] = (bitmask4 << 4) | ((codepoint >> 18) & bitmask3);
		str[1] = bit8 | ((codepoint >> 12) & bitmask6);
		str[2] = bit8 | ((codepoint >>  6) & bitmask6);
		str[3] = bit8 | ( codepoint        & bitmask6);
		inc = 4;
	}
	else{
		str[0] = '?';
		inc = 1;
	}
	return(inc);
}

internal U32
utf16_encode(U16 *str, U32 codepoint){
	U32 inc = 1;
	if (codepoint == max_U32){
		str[0] = (U16)'?';
	}
	else if (codepoint < 0x10000){
		str[0] = (U16)codepoint;
	}
	else{
		U32 v = codepoint - 0x10000;
		str[0] = safe_cast_u16(0xD800 + (v >> 10));
		str[1] = safe_cast_u16(0xDC00 + (v & bitmask10));
		inc = 2;
	}
	return(inc);
}

internal U32
utf8_from_utf32_single(U8 *buffer, U32 character){
	return(utf8_encode(buffer, character));
}

////////////////////////////////
//~ rjf: Unicode String Conversions

internal String8
str8_from_16(Arena *arena, String16 in)
{
	String8 result = str8_zero();
	if(in.size)
	{
		U64 cap = in.size*3;
		U8 *str = push_array_no_zero(arena, U8, cap + 1);
		U16 *ptr = in.str;
		U16 *opl = ptr + in.size;
		U64 size = 0;
		UnicodeDecode consume;
		for(;ptr < opl; ptr += consume.inc)
		{
			consume = utf16_decode(ptr, opl - ptr);
			size += utf8_encode(str + size, consume.codepoint);
		}
		str[size] = 0;
		arena_pop(arena, (cap - size));
		result = str8(str, size);
	}
	return result;
}

internal String16
str16_from_8(Arena *arena, String8 in)
{
	String16 result = str16_zero();
	if(in.size)
	{
		U64 cap = in.size*2;
		U16 *str = push_array_no_zero(arena, U16, cap + 1);
		U8 *ptr = in.str;
		U8 *opl = ptr + in.size;
		U64 size = 0;
		UnicodeDecode consume;
		for(;ptr < opl; ptr += consume.inc)
		{
			consume = utf8_decode(ptr, opl - ptr);
			size += utf16_encode(str + size, consume.codepoint);
		}
		str[size] = 0;
		arena_pop(arena, (cap - size)*2);
		result = str16(str, size);
	}
	return result;
}

internal String8
str8_from_32(Arena *arena, String32 in)
{
	String8 result = str8_zero();
	if(in.size)
	{
		U64 cap = in.size*4;
		U8 *str = push_array_no_zero(arena, U8, cap + 1);
		U32 *ptr = in.str;
		U32 *opl = ptr + in.size;
		U64 size = 0;
		for(;ptr < opl; ptr += 1)
		{
			size += utf8_encode(str + size, *ptr);
		}
		str[size] = 0;
		arena_pop(arena, (cap - size));
		result = str8(str, size);
	}
	return result;
}

internal String32
str32_from_8(Arena *arena, String8 in)
{
	String32 result = str32_zero(); 
	if(in.size)
	{
		U64 cap = in.size;
		U32 *str = push_array_no_zero(arena, U32, cap + 1);
		U8 *ptr = in.str;
		U8 *opl = ptr + in.size;
		U64 size = 0;
		UnicodeDecode consume;
		for(;ptr < opl; ptr += consume.inc)
		{
			consume = utf8_decode(ptr, opl - ptr);
			str[size] = consume.codepoint;
			size += 1;
		}
		str[size] = 0;
		arena_pop(arena, (cap - size)*4);
		result = str32(str, size);
	}
	return result;
}

////////////////////////////////
//~ rjf: Time Types -> String

internal String8
string_from_week_day(WeekDay week_day){
	local_persist String8 strings[] = {
		str8_lit_comp("Sun"),
		str8_lit_comp("Mon"),
		str8_lit_comp("Tue"),
		str8_lit_comp("Wed"),
		str8_lit_comp("Thu"),
		str8_lit_comp("Fri"),
		str8_lit_comp("Sat"),
	};
	String8 result = str8_lit("Err");
	if ((U32)week_day < WeekDay_COUNT){
		result = strings[week_day];
	}
	return(result);
}

internal String8
string_from_month(Month month){
	local_persist String8 strings[] = {
		str8_lit_comp("Jan"),
		str8_lit_comp("Feb"),
		str8_lit_comp("Mar"),
		str8_lit_comp("Apr"),
		str8_lit_comp("May"),
		str8_lit_comp("Jun"),
		str8_lit_comp("Jul"),
		str8_lit_comp("Aug"),
		str8_lit_comp("Sep"),
		str8_lit_comp("Oct"),
		str8_lit_comp("Nov"),
		str8_lit_comp("Dec"),
	};
	String8 result = str8_lit("Err");
	if ((U32)month < Month_COUNT){
		result = strings[month];
	}
	return(result);
}

internal String8
push_file_name_date_string(Arena *arena, DateTime *date_time)
{
	String8 result = push_str8f(arena, "%d-%02d-%02d",
								date_time->year, date_time->month + 1, date_time->day);
	return(result);
}

internal String8
push_file_name_time_sec_string(Arena *arena, DateTime *date_time)
{
	U32 adjusted_hour = date_time->hour%12;
	if (adjusted_hour == 0){
		adjusted_hour = 12;
	}
	char *ampm = "AM";
	if (date_time->hour >= 12){
		ampm = "PM";
	}
	String8 result = push_str8f(arena, "%02d.%02d.%02d %s",
								adjusted_hour, date_time->min, date_time->sec, ampm);
	return(result);
}

internal String8
push_file_name_time_string(Arena *arena, DateTime *date_time)
{
	U32 adjusted_hour = date_time->hour%12;
	if (adjusted_hour == 0){
		adjusted_hour = 12;
	}
	char *ampm = "AM";
	if (date_time->hour >= 12){
		ampm = "PM";
	}
	String8 result = push_str8f(arena, "%02d.%02d %s",
								adjusted_hour, date_time->min, ampm);
	return(result);
}

internal String8
push_file_name_date_time_string(Arena *arena, DateTime *date_time){
	String8 result = push_str8f(arena, "%d-%02d-%02d--%02d-%02d-%02d",
								date_time->year, date_time->month + 1, date_time->day,
								date_time->hour, date_time->min, date_time->sec);
	return(result);
}

internal String8
string_from_elapsed_time(Arena *arena, DateTime dt){
	Temp scratch = scratch_begin(arena);
	String8List list = {0};
	if (dt.year){
		str8_list_pushf(scratch.arena, &list, "%dy", dt.year);
		str8_list_pushf(scratch.arena, &list, "%um", dt.mon);
		str8_list_pushf(scratch.arena, &list, "%ud", dt.day);
	} else if (dt.mon){
		str8_list_pushf(scratch.arena, &list, "%um", dt.mon);
		str8_list_pushf(scratch.arena, &list, "%ud", dt.day);
	} else if (dt.day){
		str8_list_pushf(scratch.arena, &list, "%ud", dt.day);
	}
	str8_list_pushf(scratch.arena, &list, "%u:%u:%u:%u ms", dt.hour, dt.min, dt.sec, dt.msec);
	StringJoin join = { str8_lit_comp(""), str8_lit_comp(" "), str8_lit_comp("") };
	String8 result = str8_list_join(arena, &list, &join);
	scratch_end(scratch);
	return(result);
}

internal String8
string_from_guid(Arena *arena, Guid guid)
{
	String8 result = push_str8f(arena, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
								guid.data1,
								guid.data2,
								guid.data3,
								guid.data4[0],
								guid.data4[1],
								guid.data4[2],
								guid.data4[3],
								guid.data4[4],
								guid.data4[5],
								guid.data4[6],
								guid.data4[7]);
	return result;
}
