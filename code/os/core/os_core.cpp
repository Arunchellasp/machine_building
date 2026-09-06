
////////////////////////////////
//~ rjf: Handle Type Functions (Helpers, Implemented Once)

internal OS_Handle
os_handle_zero(void)
{
	OS_Handle handle = {0};
	return handle;
}

internal B32
os_handle_match(OS_Handle a, OS_Handle b)
{
	return a.u64[0] == b.u64[0];
}

internal void
os_handle_list_push(Arena *arena, OS_HandleList *handles, OS_Handle handle)
{
	OS_HandleNode *n = push_array(arena, OS_HandleNode, 1);
	n->v = handle;
	SLLQueuePush(handles->first, handles->last, n);
	handles->count += 1;
}

internal OS_HandleArray
os_handle_array_from_list(Arena *arena, OS_HandleList *list)
{
	OS_HandleArray result = {0};
	result.count = list->count;
	result.v = push_array_no_zero(arena, OS_Handle, result.count);
	U64 idx = 0;
	for(OS_HandleNode *n = list->first; n != 0; n = n->next, idx += 1)
	{
		result.v[idx] = n->v;
	}
	return result;
}

////////////////////////////////
//~ rjf: Filesystem Helpers (Helpers, Implemented Once)

internal String8
os_data_from_file_path(Arena *arena, String8 path)
{
	OS_Handle file = os_file_open(OS_AccessFlag_Read|OS_AccessFlag_ShareRead, path);
	FileProperties props = os_properties_from_file(file);
	String8 data = os_string_from_file_range(arena, file, r1u64(0, props.size));
	os_file_close(file);
	return data;
}

internal B32
os_write_data_to_file_path(String8 path, String8 data)
{
	B32 good = 0;
	OS_Handle file = os_file_open(OS_AccessFlag_Write, path);
	if(!os_handle_match(file, os_handle_zero()))
	{
		good = 1;
		os_file_write(file, r1u64(0, data.size), data.str);
		os_file_close(file);
	}
	return good;
}

internal B32
os_write_data_list_to_file_path(String8 path, String8List list)
{
	B32 good = 0;
	OS_Handle file = os_file_open(OS_AccessFlag_Write, path);
	if(!os_handle_match(file, os_handle_zero()))
	{
		good = 1;
		U64 off = 0;
		for(String8Node *n = list.first; n != 0; n = n->next)
		{
			os_file_write(file, r1u64(off, off+n->string.size), n->string.str);
			off += n->string.size;
		}
		os_file_close(file);
	}
	return good;
}

internal B32
os_append_data_to_file_path(String8 path, String8 data)
{
	B32 good = 0;
	if(data.size != 0)
	{
		OS_Handle file = os_file_open(OS_AccessFlag_Write|OS_AccessFlag_Append, path);
		if(!os_handle_match(file, os_handle_zero()))
		{
			good = 1;
			U64 pos = os_properties_from_file(file).size;
			os_file_write(file, r1u64(pos, pos+data.size), data.str);
			os_file_close(file);
		}
	}
	return good;
}

internal OS_FileID
os_id_from_file_path(String8 path)
{
	OS_Handle file = os_file_open(OS_AccessFlag_Read|OS_AccessFlag_ShareRead, path);
	OS_FileID id = os_id_from_file(file);
	os_file_close(file);
	return id;
}

internal S64
os_file_id_compare(OS_FileID a, OS_FileID b)
{
	S64 cmp = MemoryCompare((void*)&a.v[0], (void*)&b.v[0], sizeof(a.v));
	return cmp;
}

internal String8
os_string_from_file_range(Arena *arena, OS_Handle file, Rng1U64 range)
{
	U64 pre_pos = arena_pos(arena);
	String8 result;
	result.size = dim_1u64(range);
	result.str = push_array_no_zero(arena, U8, result.size);
	U64 actual_read_size = os_file_read(file, range, result.str);
	if(actual_read_size < result.size)
	{
		arena_pop_to(arena, pre_pos + actual_read_size);
		result.size = actual_read_size;
	}
	return result;
}

// TODO(rsb): Upgrade file/folder filter
internal OS_FileInfoList
file_info_list_from_path(Arena *arena, String8 path, FileSystemFilters *filters)
{
	OS_FileInfoList result = {0};
	OS_FileIter *iter = os_file_iter_begin(arena, path, filters->flags);
	for(OS_FileInfo info = {0}; os_file_iter_next(arena, iter, &info); )
	{
		b32 push;
		String8 name = info.name;
		b32 is_folder = info.props.flags & FilePropertyFlag_IsFolder;
		
		//- rsb: Filter file system entry
		if(is_folder)
		{
			//- rsb: Exclude folders
			push = 1;
			String8Array *array = &filters->exclude_folders;
			for(u64 it = 0; it < array->count; it += 1)
			{
				if(str8_match(name, array->v[it], StringMatchFlag_CaseInsensitive))
				{
					push = 0;
					break;
				}
			}
		}
		else
		{
			//- rsb: Include file extension
			push = 0;
			String8Array *array = &filters->file_extensions;
			for(u64 it = 0; it < array->count; it += 1)
			{
				if(str8_ends_with(name, array->v[it], StringMatchFlag_CaseInsensitive))
				{
					push = 1;
					break;
				}
			}
		}
		
		//- rsb: Add entry to list
		if(push)
		{
			OS_FileInfoNode *n = push_array_no_zero(arena, OS_FileInfoNode, 1);
			n->v = info;
			SLLQueuePush(result.first, result.last, n);
			result.count += 1;
		}
	}
	os_file_iter_end(iter);
	return result;
}

internal OS_FileInfoArray
file_info_array_from_path(Arena *arena, String8 path, FileSystemFilters *filter)
{
	OS_FileInfoList list = file_info_list_from_path(arena, path, filter);
	OS_FileInfoArray result = {0};
	result.v = push_array(arena, OS_FileInfo, list.count);
	result.count = list.count;
	u64 idx = 0;
	for(OS_FileInfoNode *n = list.first; n != 0; n = n->next, idx += 1)
	{
		result.v[idx] = n->v;
	}
	return result;
}
