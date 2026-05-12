#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H

#include "../types.h"

struct gdt_ptr {
    unsigned short size;
    unsigned int address;
} __attribute__((packed));

/**
 * The structure of a gdt entry
 * Limit -> max addressable unit (20 bits, 16 stored in limit_low, 4 in lower of granularity)
 * Base -> where the segment should begin (32 bits, 16 in base_low, 8 in base_mid, 8 in base_high)
 * Access -> https://wiki.osdev.org/Global_Descriptor_Table
 * Flags (high of gran.) -> https://wiki.osdev.org/Global_Descriptor_Table
 */
struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_mid;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

void load_gdt(struct gdt_ptr* table);
void load_ltr(uint32_t selector);
void gdt_init();
void set_tss_stack_pointer(uint32_t *esp0);

#endif
