# Startup Sequence

## 1. Load from GRUB and jump into C

In this step, we perform the following:

1. Load from GRUB
2. Initialise a statically-defined boot page directory, and one boot page table for the for directory entry.
3. Identity map the first directory entry so that we can enable paging.
4. Enable paging using this paging directory
5. Perform a long jump to the higher half
6. Remove the old identity mapping (but retain the higher half mapping for the first directory entry)
7. Push kernel start, end values to the stack and then `call` the `kmain` C function.

## 2. Initialise paging

After jumping from the ASM setup, we only have a higher-half mapping for the first directory entry. We need to figure out how much more space the kernel takes in memory and then map it.

1. Perform a call to `init_paging()`.
2. Calculate the memory we need to map for the kernel:
    1. Take the `kernel_physical_end` value we pushed to the stack earlier
    2. Take the information from the GRUB multiboot header, such as module locations, and structure information
    3. Whatever value we find from these that is highest, we will use as the upper limti of the kernel.
3. Using this value, we go through the number of page directory and page table entries we need to, and map the 3GB+ addresses to their lower half positions. The kernel directory is stored as a `static pde_t kernel_pde[1024]` inside of mem.c.
4. Now, load the paging directory we just filled in.
5. After, we can finally go back through the frame bitmap and mark all the locations of the physical kernel as occupied. Previously we couldn't do this, because the bitmap was not in mapped memory.

## 3. Initialise GDT

At this point, we have successfully enabled paging and have mapped out all the kernel so we should be able to run C code without issues. Next, we need to setup the GDT so that we can
