
thread_static TCTX *tctx_thread_local = 0;

internal void
tctx_init_and_equip(TCTX *tctx)
{
	MemoryZeroStruct(tctx);
	for(u64 it = 0; it < ArrayCount(tctx->arenas); it += 1)
	{
		tctx->arenas[it] = arena_alloc_default();
	}
	tctx_thread_local = tctx;
}

internal void
tctx_release(void)
{
	for(u64 it = 0; it < ArrayCount(tctx_thread_local->arenas); it += 1)
	{
		arena_release(tctx_thread_local->arenas[it]);
	}
	MemoryZeroStruct(tctx_thread_local);
	tctx_thread_local = 0;
}

internal TCTX *
tctx_get_equipped(void)
{
	return tctx_thread_local;
}

internal Arena *
tctx_get_scratch(Arena *conflicts)
{
	Arena *result = 0;
	TCTX *tctx = tctx_get_equipped();
	for(u64 it = 0; it < ArrayCount(tctx->arenas); it += 1)
	{
		Arena *arena = tctx->arenas[it];
		if(conflicts != arena)
		{
			result = arena;
			break;
		}
	}
	return result;
}
