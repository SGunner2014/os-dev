#include "gdt.h"
#include "process.h"

static struct gdt_entry gdt[6];
static struct gdt_ptr gp;

static Tss_Entry tss_entry;

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

void create_tss_gdt(uint32_t i)
{
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = sizeof(tss_entry);
    uint8_t access = 0x89; // 1000 1001
    uint8_t gran = 0;

    uint32_t *tss = (uint32_t*) &tss_entry;

    for (uint32_t j = 0; j < sizeof(tss_entry); j++) {
        tss[j] = 0;
    }

    set_entry(i, limit, base, access, gran);
}

void gdt_init()
{
    gp.size = (sizeof(struct gdt_entry) * 5) - 1;
    gp.address = (unsigned int) &gdt;

    set_entry(0, 0, 0, 0, 0);
    set_entry(1, 0xffffffff, 0x0, 0x9a, 0xcf); // code segment for kernel
    set_entry(2, 0xffffffff, 0x0, 0x92, 0xcf); // data segment for kernel
    set_entry(3, 0xffffffff, 0x0, 0xFE, 0xcf); // code segment for user
    set_entry(4, 0xffffffff, 0x0, 0xF2, 0xcf); // data segment for user

    create_tss_gdt(5);

    load_gdt(&gp);
}
