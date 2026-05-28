# Loading an ELF Executable

We have a couple of ways to load an ELF executable:

1. We load it via a GRUB module
2. We load it from the filesystem

For the moment, only #1 is supported because we don't have a filesystem.

## Loading from GRUB

- Grub will already automatically load the exeuctable into memory.
