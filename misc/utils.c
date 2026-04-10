#include "utils.h"

unsigned int strlen(const char *buff)
{
    unsigned int i = 0;
    while (buff[i] != '\0') {
        i++;
    }

    return i;
}
