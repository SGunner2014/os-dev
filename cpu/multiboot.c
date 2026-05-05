#include "multiboot.h"
#include "../misc/utils.h"

struct sysinfo read_multiboot(struct multiboot_info *multiboot_str)
{
    struct sysinfo info;
    info.struct_addr = (uint32_t) multiboot_str;
    multiboot_str = (struct multiboot_info*)((uint32_t)multiboot_str + 0xC0000000);

    info.total_memory = (multiboot_str->mem_lower) + (multiboot_str->mem_upper);

    return info;
}
