#include "gdt.h"

static struct gdt_entry gdt[3];
static struct gdt_ptr gp;

static void set_entry(
    int i, unsigned int limit, unsigned int base, unsigned char access,
    unsigned char gran)
{
    gdt[i].granularity = (gran & 0xf0) | ((limit >> 16) & 0x0f);
    gdt[i].limit_low = (limit & 0xffff);
    gdt[i].base_low = (base & 0xffff);
    gdt[i].base_mid = (base >> 16) & 0xff;
    gdt[i].base_high = (base >> 24) & 0xff;
    gdt[i].access = access;
}

/**
 * 10011011 = 0x9b
 * 10010011 = 0x93
 */

void gdt_init()
{
    gp.size = (sizeof(struct gdt_entry) * 3) - 1;
    gp.address = (unsigned int) &gdt;

    set_entry(0, 0, 0, 0, 0);
    set_entry(1, 0xffffffff, 0x0, 0x9a, 0xcf); // code segment for kernel
    set_entry(2, 0xffffffff, 0x0, 0x92, 0xcf); // data segment for kernel
    // set_entry(3, 0xffffffff, 0x0, 0xFE, 0xcf); // code segment for user
    // set_entry(4, 0xffffffff, 0x0, 0xF2, 0xcf); // data segment for user

    load_gdt(&gp);
}
