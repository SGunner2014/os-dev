#ifndef INCLUDE_WRITE_H
#define INCLUDE_WRITE_H

#include "../../types.h"
#include "../../drivers/vga.h"

void write(char *buff, uint32_t size)
{
    print(buff);
}

#endif
