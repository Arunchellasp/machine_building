
internal Ring_Buffer 
init_ring_buffer(Arena *arena, u32 step_size, u32 num_steps)
{
    Ring_Buffer buffer = {0};
    buffer.data = (void *)push_array(arena, u8, step_size * num_steps * 64);
    buffer.buffer_size = step_size * num_steps;
    buffer.step_size = step_size;
    buffer.num_entries_inserted = 0;
	return buffer;
}

#define push_ring_buffer(buffer, data) push_ring_buffer_(buffer, (void *)data, sizeof(*data))
internal void 
push_ring_buffer_(Ring_Buffer *buffer, void *data, u32 step_size)
{
    Assert(step_size == buffer->step_size);
    MemoryCopy((u8 *)((u8 *)buffer->data + buffer->next_write_idx), (u8 *)data, step_size);
    buffer->next_write_idx = (buffer->next_write_idx + buffer->step_size) % buffer->buffer_size;
	buffer->num_entries_inserted++;
}

internal void *
get_recent_entry(Ring_Buffer *buffer)
{
    u32 recent_entry_idx = (buffer->buffer_size + buffer->next_write_idx - buffer->step_size) % buffer->buffer_size;
    return (void *)((u8 *)buffer->data + recent_entry_idx);
}

internal void *
pop_ring_buffer(Ring_Buffer *buffer)
{
	void *result = (void *)((u8 *)buffer->data + buffer->next_read_idx);
	buffer->next_read_idx = (buffer->next_read_idx + buffer->step_size) % buffer->buffer_size;
	return result;
}

internal void
copy_buffer(Ring_Buffer *to, Ring_Buffer *from)
{
	while(to->num_entries_inserted < from->num_entries_inserted)
	{
		void *data = pop_ring_buffer(from);
		push_ring_buffer_(to, data, from->step_size);
	}
}
