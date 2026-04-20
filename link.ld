ENTRY(loader)

SECTIONS {
  . = 0xC0100000; /* Stored at 3GB + 1MB in memory */

  _kernel_start = .;
  _kernel_physical_start = . - 0xC0000000;

  .multiboot ALIGN(4) : AT(ADDR(.multiboot)-0xC0000000)
  {
    *(.multiboot)
  }

  .text ALIGN (0x1000) : AT(ADDR(.text)-0xC0000000)
  {
      *(.text)
  }

  .rodata ALIGN (0x1000) : AT(ADDR(.rodata)-0xC0000000)
  {
      *(.rodata*)
  }

  .data ALIGN (0x1000) : AT(ADDR(.data)-0xC0000000)
  {
    *(.data)
  }

  .bss ALIGN (0x1000) : AT(ADDR(.bss)-0xC0000000)
  {
    *(COMMON)
    *(.bss)
  }

  _kernel_end = .;
  _kernel_physical_end = . - 0xC0000000;
}
