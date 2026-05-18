#include "cpu/process.h"
#include "io.h"
#include "screen.h"
#include "cpu/gdt.h"
#include "cpu/mem.h"
#include "idt.h"
#include "misc/utils.h"
#include "cpu/multiboot.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8
/* IO ports */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

/* IO port commands */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15


char *fb = (char *) 0xC00B8000;

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

void handle_page_fault(struct cpu_state *cpu)
{
    UNUSED(cpu);

    char buff[64];
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

    prints("\n-- Page fault --\n");

    itoa(cr2, buff, 16);
    prints("Address: ");
    prints(buff);
    prints("\n");

    // Process *process = get_current_process();

    // itoa((uint32_t) process->prog, buff, 16);
    // prints(buff);


    for (;;);
}

void handle_syscall(struct cpu_state *cpu)
{
    UNUSED(cpu);
    prints("syscall\n");
}

void kmain(
    unsigned int kernel_physical_end,
    unsigned int kernel_end,
    unsigned int kernel_physical_start,
    unsigned int kernel_start,
    multiboot_info_t *multiboot_addr
)
{
    clear_screen();
    init_paging(
        kernel_start,
        kernel_physical_start,
        kernel_end,
        kernel_physical_end,
        multiboot_addr
    );

    write("Hello", 5);

    for (;;) ;

    // char abuff[64];
    // prints("kernel_start="); itoa(kernel_start, abuff, 16); prints(abuff); prints("\n");
    // prints("kernel_end="); itoa(kernel_end, abuff, 16); prints(abuff); prints("\n");
    // prints("kernel_physical_start="); itoa(kernel_physical_start, abuff, 16); prints(abuff); prints("\n");
    // prints("kernel_physical_end="); itoa(kernel_physical_end, abuff, 16); prints(abuff); prints("\n");

    // struct sysinfo info = read_multiboot(multiboot_addr);
    // UNUSED(multiboot_addr);
    // UNUSED(info);

    // fb_move_cursor(0);

    // // Load gdt
    // gdt_init();
    // prints("Set GDT\n");
    // // Load interrupts
    // idt_init();
    // prints("Set IDT\n");

    // prints("Multiboot structure was at: 0x");
    // char buff[64];
    // itoa(info.struct_addr, buff, 16);
    // prints(buff);
    // prints("\n");

    // prints("Boot complete, total memory detected: ");
    // char memstr[256];
    // format_memory_str(info.total_memory, memstr);
    // prints(memstr);
    // prints("\n");

    // init_keyboard();
    // register_interrupt_handler(0x0E, handle_page_fault);
    // register_interrupt_handler(0x80, handle_syscall);

    // __asm__("int $0x80");

    // multiboot_module_t *mod = (multiboot_module_t*) (uint32_t) (multiboot_addr->mods_addr + PHYS_OFFSET);

    // Process *process = create_process(mod->mod_start + PHYS_OFFSET, mod->mod_end - mod->mod_start + 1);

    // prints("Created process\n");

    // UNUSED(process);

    // if (multiboot_addr->mods_count == 1)
    // {
    //     switch_context(process);

    //     prints("Exec process\n");
    //     exec_process(process);
    // }


    // for (;;) ;
}
