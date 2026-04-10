OBJECTS = loader.o kmain.o io.o screen.o gdt_asm.o gdt.o idt.o idt_asm.o misc/utils.o drivers/keyboard.o
CC = x86_64-elf-gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
				 -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
					-Wno-pointer-to-int-cast
LD = x86_64-elf-ld
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf32

all: kernel.elf

kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	xorriso -as mkisofs -R \
				  -b boot/grub/stage2_eltorito \
					-no-emul-boot \
					-boot-load-size 4 \
					-A os \
					-input-charset utf8 \
					-quiet \
					-boot-info-table \
					-o os.iso \
					iso

run: os.iso
	qemu-system-x86_64 -cdrom os.iso -m 32

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	find . -name "*.o" -delete
	rm -f kernel.elf os.iso
