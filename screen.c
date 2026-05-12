#include "screen.h"
#include "misc/utils.h"

#define BLACK 0x0
#define WHITE 0xf
#define WHITE_ON_BLACK (BLACK) << 4 | (WHITE)

const unsigned int VGA_WIDTH = 80;
const unsigned int VGA_HEIGHT = 25;
const unsigned int MAX_POS = VGA_WIDTH * VGA_HEIGHT - 1;

static char *fb = (char*) 0xC00B8000;

static void scroll_screen();

/**
 * Prints a character to the screen
 */
void printc(char chr)
{
    static unsigned int pos = 0;

    if (chr != '\n') {
        fb[pos * 2] = chr;
        fb[pos * 2 + 1] = (BLACK) << 4 | WHITE;
    }

    // If newline char, then move to the next line
    if (chr == '\n') {
        unsigned char pos_l = pos / VGA_WIDTH;
        pos = (pos_l + 1) * VGA_WIDTH;
    } else {
        pos++;
    }

    // If we've gone past the end of the screen, scroll the screen
    if (pos > MAX_POS) {
        scroll_screen();
        pos = ((VGA_HEIGHT - 1) * VGA_WIDTH);
    }
}

/**
 * Prints a string to the screen
 */
void prints(char *buff)
{
    unsigned int len = strlen(buff);
    // unsigned int len = 3;

    for (unsigned int i = 0; i < len; i++)
    {
        printc(buff[i]);
    }
}

void clear_screen()
{
    for (unsigned int i = 0; i < MAX_POS + 1; i++)
    {
        fb[i * 2] = ' ';
        fb[i * 2 + 1] = WHITE_ON_BLACK;
    }
}

/**
 * Scrolls the screen one line
 */
static void scroll_screen()
{
    // char *fb = (char *) 0x000B8000;

    // Move everything a line up
    for (unsigned int y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (unsigned int x = 0; x < VGA_WIDTH - 1; x++)
        {
            fb[(y * VGA_WIDTH + x) * 2] = fb[((y + 1) * VGA_WIDTH + x) * 2];
            fb[(y * VGA_WIDTH + x) * 2 + 1] = fb[((y + 1) * VGA_WIDTH + x) * 2 + 1];
        }
    }

    // Clear the last line
    for (unsigned int i = 0; i < VGA_WIDTH; i++)
    {
        fb[((VGA_HEIGHT - 1) * VGA_WIDTH * 2) + (i * 2)] = ' ';
        fb[((VGA_HEIGHT - 1) * VGA_WIDTH * 2) + (i * 2 + 1)] = 0x0;
    }
}

void printui(uint32_t i, uint32_t base)
{
    char buff[16];

    itoa(i, buff, base);
    prints(buff);
}
