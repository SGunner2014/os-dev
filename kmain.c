#include "cpu/gdt.h"
#include "cpu/mem.h"
#include "cpu/multiboot.h"
#include "cpu/process.h"
#include "cpu/syscall.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "idt.h"
#include "io.h"
#include "misc/elf.h"
#include "misc/utils.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8
/* IO ports */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

/* IO port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

char *fb = (char *)0xC00B8000;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    fb[i] = c;
    fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

/**
 * Moves the cursor to the given position
 */
void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

static uint32_t calculate_free_space_beginning(uint32_t kernel_phys_end,
                                               multiboot_info_t *multiboot)
{
    uint32_t addr = kernel_phys_end;

    multiboot_module_t *mod;
    uint32_t i;

    // Take the end address of each module in case it's larger than the end of
    // the kernel physical
    for (i = 0, mod = (multiboot_module_t *)(uint32_t)(multiboot->mods_addr +
                                                       PHYS_OFFSET);
         i < multiboot->mods_count; i++, mod++) {
        addr = mod->mod_end > addr ? mod->mod_end : addr;

        uint32_t mod_end = ((uint32_t)mod) - PHYS_OFFSET;
        mod_end = mod_end + sizeof(multiboot_module_t);
        addr = mod_end > addr ? mod_end : addr;
    }

    uint32_t multiboot_end =
        (((uint32_t)multiboot) - PHYS_OFFSET) + sizeof(multiboot_info_t);
    addr = multiboot_end > addr ? multiboot_end : addr;

    // Now, make it page-aligned
    return addr % PAGE_SIZE == 0 ? addr : ((addr / PAGE_SIZE) + 1) * PAGE_SIZE;
}

void handle_page_fault(struct cpu_state *cpu, struct stack_state *stack)
{
    UNUSED(cpu);
    UNUSED(stack);

    char buff[64];
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

    print("\n-- Page fault --\n");

    itoa(cr2, buff, 16);
    print("Address: ");
    print(buff);
    print("\n");

    itoa(stack->error_code, buff, 16);
    print("Code: ");
    print(buff);
    print("\n");

    // Process *process = get_current_process();

    // itoa((uint32_t) process->prog, buff, 16);
    // print(buff);

    for (;;)
        ;
}

void kmain(unsigned int kernel_physical_end, unsigned int kernel_end,
           unsigned int kernel_physical_start, unsigned int kernel_start,
           multiboot_info_t *multiboot_addr)
{
    UNUSED(calculate_free_space_beginning);

    // Calculate end of kernel, beginning of free space
    uint32_t free_space_phys = calculate_free_space_beginning(
        kernel_physical_end, (multiboot_info_t *)((uint32_t)multiboot_addr));

    clear_screen();
    k_init_paging(free_space_phys);

    write("Hello", 5);

    char abuff[64];
    print("kernel_start=");
    itoa(kernel_start, abuff, 16);
    print(abuff);
    print("\n");
    print("kernel_end=");
    itoa(kernel_end, abuff, 16);
    print(abuff);
    print("\n");
    print("kernel_physical_start=");
    itoa(kernel_physical_start, abuff, 16);
    print(abuff);
    print("\n");
    print("kernel_physical_end=");
    itoa(kernel_physical_end, abuff, 16);
    print(abuff);
    print("\n");

    struct sysinfo info = read_multiboot(multiboot_addr);
    UNUSED(multiboot_addr);
    UNUSED(info);

    fb_move_cursor(0);

    // Load gdt
    gdt_init();
    print("Set GDT\n");
    // Load interrupts
    idt_init();
    print("Set IDT\n");

    print("Multiboot structure was at: 0x");
    char buff[64];
    itoa(info.struct_addr, buff, 16);
    print(buff);
    print("\n");

    print("Boot complete, total memory detected: ");
    char memstr[256];
    format_memory_str(info.total_memory, memstr);
    print(memstr);
    print("\n");

    init_keyboard();
    register_interrupt_handler(0x0E, handle_page_fault);
    init_syscalls();

    multiboot_module_t *mod =
        (multiboot_module_t *)(uint32_t)(multiboot_addr->mods_addr +
                                         PHYS_OFFSET);

    // Process *process = create_process(mod->mod_start + PHYS_OFFSET,
    // mod->mod_end - mod->mod_start + 1);
    Process *process =
        elf_load_file((uint32_t *)(uint32_t)(mod->mod_start + PHYS_OFFSET));

    print("Created process\n");

    itoa((uint32_t)process, buff, 16);
    print("Process addr: ");
    print(buff);
    print("\n");

    itoa((uint32_t)process->prog, buff, 16);

    print("Prog addr: ");
    print(buff);
    print("\n");

    if (multiboot_addr->mods_count == 1) {
        print("Switching context\n");
        switch_context(process);

        print("Exec process\n");
        exec_process(process);
    }

    for (;;)
        ;
}
