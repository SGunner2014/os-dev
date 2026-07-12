# Loading an ELF Executable

We have a couple of ways to load an ELF executable:

1. We load it via a GRUB module
2. We load it from the filesystem

For the moment, only #1 is supported because we don't have a filesystem.

## Loading from GRUB

- Grub will already automatically load the exeuctable into memory.


## Changes needed

- Currently when we load an elf file in from grub, we're not relocating the code in memory. Instead, we just allocate the virtual addresses needed and then map them to the physical location in ram where grub loaded them. 
- Instead, we should allocate the memory, zero it, and then relocate the code. 