#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

/**
 * Sends given data to an io port. Defined in io.s
 *
 * @param port
 * @param data
 */
void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

#endif
