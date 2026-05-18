#include "vga.h"
#include "../misc/utils.h"

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define BLACK 0x0
#define WHITE 0xf
#define WHITE_ON_BLACK ((BLACK) << 4 | (WHITE))
#define MAX_POS (VGA_WIDTH * VGA_HEIGHT - 1)

static char *fb = (char*) 0xC00B8000;
static uint32_t char_pos = 0;

static void scroll_screen()
{
    for (uint32_t i = 0; i < VGA_WIDTH; i++) {
        for (uint32_t j = 0; j < VGA_HEIGHT - 1; j++) {
            uint32_t l_offset = ((j * VGA_HEIGHT) + i) * 2;
            uint32_t h_offset = ((j * (VGA_HEIGHT + 1)) + i) * 2;
            fb[l_offset] = fb[h_offset];
            fb[l_offset + 1] = fb[h_offset + 1];
        }
    }

    for (uint32_t i = 0; i < VGA_WIDTH; i++) {
        uint32_t offset = (((VGA_HEIGHT - 1) * VGA_WIDTH) * 2);
        fb[offset] = 0;
        fb[offset + 1] = 0;
    }
}

static uint32_t vert(uint32_t pos)
{
    return pos / VGA_WIDTH;
}

static void writec(char chr)
{
    // Stores the current position
    uint32_t line = vert(char_pos);

    switch (chr) {
        case '\n':
            if (line == VGA_HEIGHT - 1) { // On last line of screen
                scroll_screen();
                char_pos = (VGA_HEIGHT - 1) * VGA_WIDTH;
            } else {
                char_pos = (line + 1) * VGA_WIDTH;
            }
            break;
        case '\r':
            char_pos = line * VGA_WIDTH;
            break;
        case '\b':
            if (char_pos > 0)
                char_pos--;
            break;
        default:
            fb[char_pos * 2] = chr;
            fb[char_pos * 2 + 1] = WHITE_ON_BLACK;

            if (++char_pos > MAX_POS) {
                scroll_screen();
                char_pos = (VGA_HEIGHT - 1) * VGA_WIDTH;
            }
    }
}

void write(char *buff, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        writec(buff[i]);
    }
}

void clear_screen()
{
    for (uint32_t i = 0; i < VGA_HEIGHT; i++) {
        for (uint32_t j = 0; j < VGA_WIDTH; j++) {
            uint32_t offset = ((i * VGA_WIDTH) + j) * 2;
            fb[offset] = 0;
            fb[offset + 1] = 0;
        }
    }
}

void print(char *buff)
{
    uint32_t len = strlen(buff);
    write(buff, len);
}
