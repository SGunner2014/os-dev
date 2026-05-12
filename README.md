# An operating system, written for x86 protected mode

This is an operating system written for x86 in protected mode. The end goal is to have a system that can load programs from disk into memory, as well as a functioning shell.

I'm writing this without the help of AI - except to explain hard-to-understand concepts. All code has been written by myself, partly as a learning exercise, and partly just to enjoy coding again.

Currently, we have a base upon which to build - the following have been implemented:

- GDT
- IDT
- Paging
- Virtual Memory
- Loading programs from grub modules
- User mode

## Task list:

[] Refactor memory management so it's more readable
[] Produce documentation describing the overall architecture
[] Implement system to share common kernel functionality with processes (e.g. libraries)
[] Port mlibc
[] Implement basic shell
