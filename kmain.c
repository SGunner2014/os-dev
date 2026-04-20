#include "io.h"
#include "misc/utils.h"
#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "drivers/keyboard.h"

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

void kmain(
    unsigned int kernel_physical_end,
    unsigned int kernel_end,
    unsigned int kernel_physical_start,
    unsigned int kernel_start
)
{
    UNUSED(kernel_start);
    UNUSED(kernel_physical_start);
    UNUSED(kernel_end);
    UNUSED(kernel_physical_end);

    fb_move_cursor(0);
    clear_screen();
    prints("Hello, world!\n");

    // Load gdt
    gdt_init();
    prints("Set GDT\n");
    // Load interrupts
    idt_init();
    prints("Set IDT\n");

    init_keyboard();
}
