#include "../types.h"

#ifndef INCLUDE_MULTIBOOT_H
#define INCLUDE_MULTIBOOT_H

struct multiboot_struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms_1;
    uint32_t syms_2;
    uint32_t syms_3;
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_count;
    uint32_t drives_addr;
    uint32_t conf_table;
    uint32_t bootloader_name;
} __attribute__((packed));

struct sysinfo {
    uint32_t total_memory;
    uint32_t struct_addr;
};

struct sysinfo read_multiboot(struct multiboot_struct *multiboot_str);

#endif
