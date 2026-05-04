#include "utils.h"

unsigned int strlen(const char *buff)
{
    unsigned int i = 0;
    while (buff[i] != '\0') {
        i++;
    }

    return i;
}

void strrev(char *buff)
{
    uint32_t len = strlen(buff);

    for (uint32_t i = 0; i < len / 2; i++)
    {
        char left = buff[i];
        buff[i] = buff[len - i - 1];
        buff[len - i - 1] = left;
    }
}

void itoa(const int32_t j, char *buff, const uint32_t base)
{
    uint32_t i = (uint32_t) j;
    if (i == 0) {
        strcpy(buff, "0\0");
        return;
    }

    char *buff2 = buff;

    // bool is_negative = i < 0;
    bool is_negative = false;
    uint32_t copy = is_negative ? -i : i;

    while (copy > 0) {
        uint8_t digit = copy % base;
        char digit_a;

        if (digit >= 10) {
            digit_a = digit + 65 - 10; // 65 = 'A' in ascii
        } else {
            digit_a = digit + 48; // 48 = '0' in ascii
        }

        *buff2 = digit_a;
        buff2++;

        copy /= base;
    }

    if (is_negative) {
        *buff2 = '-';
        buff2++;
    }

    *buff2 = '\0';

    // Reverse string
    strrev(buff);
}

char *strcpy(char *buff1, const char *buff2)
{
    uint32_t len = strlen(buff2);
    for (uint32_t i = 0; i < len + 1; i++) {
        buff1[i] = buff2[i];
    }

    return buff1;
}

void format_memory_str(uint32_t kb, char *buff)
{
    UNUSED(kb);
    UNUSED(buff);

    uint32_t gb = kb / 1024 / 1024;
    kb %= 1024 * 1024;
    uint32_t mb = kb / 1024;
    kb %= 1024;

    char buff2[32];
    int buff2len;
    char *buff_cpy = buff;

    itoa(gb, buff2, 10);
    buff2len = strlen(buff2);
    strcpy(buff_cpy, buff2);
    buff_cpy += buff2len;
    strcpy(buff_cpy, "GB, \0");
    buff_cpy += 4;

    itoa(mb, buff2, 10);
    buff2len = strlen(buff2);
    strcpy(buff_cpy, buff2);
    buff_cpy += buff2len;
    strcpy(buff_cpy, "MB, \0");
    buff_cpy += 4;

    itoa(kb, buff2, 10);
    buff2len = strlen(buff2);
    strcpy(buff_cpy, buff2);
    buff_cpy += buff2len;
    strcpy(buff_cpy, "KB\0");
}
