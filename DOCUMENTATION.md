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

At this point, we have successfully enabled paging and have mapped out all the kernel so we should be able to run C code without issues. Next, we need to setup the GDT so that we can can start operating from kernel space and lay down the framework for user space further down the line.

1. We store the system GDT in a `static struct gdt_entry gdt[6]`.
2. We also store a static pointer to the gdt.
3. We set the size and address properties of the GDT pointer.
4. We then create 6 entries within the GDT:
   1. The null selector
   2. A segment each for kernel code/data
   3. A segment each for user code/data
   4. A TSS selector - this will be used when jumping to user mode, and will allow us to enable interrupts in user mode later.
5. Load the gdt pointer using `lgdt`
6. Set the new data segment registers, and then perform a far jump to set the code segment register.
7. Next, load the task segment selector for the kernel.

## 4. Initialise IDT

We've enabled paging and setup the GDT. Next, we want to setup the Interrupt Descriptor Table so that we can receive and respond to interrupts.

1. Much like the GDT, we store the IDT as a static 256-length array of `idt_entry`.
2. First, we set the size and address proeprties of the IDT pointer.
3. Next, we initialise the first 48 entries of the IDT:
   1. The first 32 entries will map to the 32 hardware interrupts
   2. The next 16 entries will be responsible for the 16 x86 PIC IRQs
4. Next, we initialise the PICs (Programmable Interrupt Controllers) - there are two on x86, and they're chained as master and slave. We remap the IRQ vectors so that they map to interrupt no.s 32+.
5. Next, tell the master pic about the slave, and tell the slave its identity is 2.
6. Next, inform the pics they should be operating in 8086 mode.
7. Then, temporarily mask all IRQs for now so that they don't fire - these will be unmasked as they're implemented.
8. Finally, make a call to `lidt` to load the IDT as we did with the GDT.

### Notes

- Different interrupts are treated differently - some are error codes (e.g. page fault), and some aren't (e.g. Divide Error, Overflow).
- Each interrupt will therefore be handled by either an error code handler, or a no error code handler.
- Each handler will then call the common handler stub with the interrupt number.
- The common handler will then look to see if we've previously registered a C handler for this interrupt and, if so, will call the handler with a pointer to a struct representing the current cpu state (registers, etc.)

## 5. Initialise keyboard

We previously initialised interrupts and registered a handler for the keyboard IRQ. Now we need some way to handle these events and to respond to them.
