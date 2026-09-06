
#ifndef BASE_THREAD_CONTEXT_H
#define BASE_THREAD_CONTEXT_H

#define thread_static __declspec(thread)

struct TCTX
{
	Arena *arenas[2];
};

internal void tctx_init_and_equip(TCTX *tctx);
internal void tctx_release(void);
internal TCTX *tctx_get_equipped(void);
internal Arena *tctx_get_scratch(Arena *conflicts);

#define scratch_begin(conflicts) temp_begin(tctx_get_scratch(conflicts))
#define scratch_end(scratch) temp_end(scratch)

internal void tctx_init_and_equip(TCTX *tctx);

#endif //BASE_THREAD_CONTEXT_H
