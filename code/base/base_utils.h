#ifndef BASE_UTILS_H

#define sort_ascending_order(a, b) ((a) > (b)) ? 1 : -1
#define sort_descending_order(a, b) ((a) < (b)) ? 1 : -1
#define quick_sort(ptr, count, element_size, cmp_function) qsort((ptr), (count), (element_size), (int (*)(const void *, const void *))(cmp_function))

//
// NOTE(rsb): @Checksums
//

internal u32 
sdbm_checksum(String8 data)
{
    u32 checksum = 0;
    while(data.size--)
	{
        checksum = checksum * 65599 + *data.str++;
    }
    return checksum;
}

//
// NOTE(rsb): @Ring_Buffer
//

struct Ring_Buffer{
    void *data;
    u32 buffer_size;
    u32 step_size;
    u32 next_read_idx;
    u32 next_write_idx;
	u32 num_entries_inserted;
};

#define BASE_UTILS_H
#endif //BASE_UTILS_H
