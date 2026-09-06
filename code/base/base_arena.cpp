
//
//~ rsb: arena creation / destruction
//

internal Arena *
arena_alloc(Arena_Params params)
{
	u64 reserve_size = params.reserve_size;
	u64 commit_size = params.commit_size;
	reserve_size = AlignPow2(reserve_size, os_get_system_info()->page_size);
	commit_size = AlignPow2(commit_size, os_get_system_info()->page_size);
	
	//- rsb: current version of arena only supports same size of reserve and commit size
	Assert(reserve_size == commit_size);
	void *base = os_reserve(reserve_size);
	os_commit(base, commit_size);
	
	Arena *arena    = (Arena *)base;
	arena->pos      = ARENA_HEADER_SIZE;
	arena->cmt      = commit_size;
	arena->res      = reserve_size;
	arena->res_size = params.reserve_size;
	arena->cmt_size = params.commit_size;
	return arena;
}

internal void
arena_release(Arena *arena)
{
	os_release((void *)arena, arena->res);
}

//
//~ rsb: arena push/pop/pos core functions
//

internal void *
arena_push(Arena *arena, u64 size, u64 align)
{
	Assert(IsPow2(align));
	u64 pos_pre = AlignPow2(arena->pos, align);
	u64 pos_pst = pos_pre + size;
	
	Assert(pos_pst <= arena->cmt);
	void *result = (u8 *)arena + pos_pre;
	arena->pos = pos_pst;
	return result;
}

internal void
arena_pop_to(Arena *arena, u64 pos)
{
	arena->pos = ClampBot(ARENA_HEADER_SIZE, pos);
}

internal u64
arena_pos(Arena *arena)
{
	return arena->pos;
}

//
//~ rsb: arena push/pop helpers
//

internal void
arena_clear(Arena *arena)
{
	arena_pop_to(arena, 0);
}

internal void
arena_pop(Arena *arena, u64 amt)
{
	u64 pos_old = arena_pos(arena);
	u64 pos_new = pos_old;
	if(amt < pos_old)
	{
		pos_new = pos_old - amt;
	}
	arena_pop_to(arena, pos_new);
}

//
//~ rsb: temporary arena scopes
//

internal Temp
temp_begin(Arena *arena)
{
	Temp temp;
	temp.arena = arena;
	temp.pos = arena->pos;
	return temp;
}

internal void
temp_end(Temp temp)
{
	temp.arena->pos = temp.pos;
}
