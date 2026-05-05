#include "io.h"
#include "screen.h"
#include "cpu/gdt.h"
#include "cpu/process.h"
#include "cpu/mem.h"
#include "idt.h"
#include "misc/utils.h"
#include "cpu/multiboot.h"
#include "drivers/keyboard.h"
#include "cpu/process.h"
#include "cpu/malloc.h"

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

    prints("Page fault\n");

    for (;;);
}

void kmain(
    unsigned int kernel_physical_end,
    unsigned int kernel_end,
    unsigned int kernel_physical_start,
    unsigned int kernel_start,
    struct multiboot_struct *multiboot_addr
)
{
    clear_screen();
    init_paging(
        kernel_start,
        kernel_physical_start,
        kernel_end,
        kernel_physical_end
    );

    char abuff[64];
    prints("kernel_start="); itoa(kernel_start, abuff, 16); prints(abuff); prints("\n");
    prints("kernel_end="); itoa(kernel_end, abuff, 16); prints(abuff); prints("\n");
    prints("kernel_physical_start="); itoa(kernel_physical_start, abuff, 16); prints(abuff); prints("\n");
    prints("kernel_physical_end="); itoa(kernel_physical_end, abuff, 16); prints(abuff); prints("\n");

    struct sysinfo info = read_multiboot(multiboot_addr);
    UNUSED(multiboot_addr);
    UNUSED(info);

    fb_move_cursor(0);
    prints("Hello, world!\n");

    // Load gdt
    gdt_init();
    prints("Set GDT\n");
    // Load interrupts
    idt_init();
    prints("Set IDT\n");

    prints("Multiboot structure was at: 0x");
    char buff[64];
    itoa(info.struct_addr, buff, 16);
    prints(buff);
    prints("\n");

    prints("Boot complete, total memory detected: ");
    char memstr[256];
    format_memory_str(info.total_memory, memstr);
    prints(memstr);
    prints("\n");

    init_keyboard();
    register_interrupt_handler(0x0E, handle_page_fault);

    prints("Before");
    Process *process = create_process();
    switch_process(process);

    prints("Switched process");


    char *test = (char*) malloc(sizeof(char));
    prints("Malloc");
    itoa((uint32_t) test, buff, 16);
    prints("Allocated virt: ");
    prints(buff);
    prints("\n");

    *test = 16;
    if (*test == 16)
    {
        prints("Malloc test worked\n");
    }


    for (;;) ;
}
