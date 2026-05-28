PLATFORM = $(shell uname)

OBJECTS = loader.o kmain.o io.o cpu/gdt_asm.o cpu/gdt.o idt.o idt_asm.o misc/utils.o drivers/keyboard.o cpu/mem.o cpu/mem_asm.o cpu/multiboot.o cpu/malloc.o cpu/process.o cpu/user_mode.o drivers/vga.o cpu/syscall.o misc/elf.o
# CC = x86_64-elf-gcc

ifeq ($(PLATFORM), Darwin)
	CC=x86_64-elf-gcc
else
	CC=gcc
endif

ifeq ($(PLATFORM), Darwin)
	LD=x86_64-elf-ld
else
	LD=ld
endif

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
				 -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
					-Wno-pointer-to-int-cast
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf32

programs/program.elf:
	$(AS) -f elf32 programs/program.nasm -o programs/program.o
	$(LD) -melf_i386 programs/program.o -o programs/program.elf

all: kernel.elf

disk.img:
	qemu-img create disk.img 512M

kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf programs/program.elf
	cp kernel.elf iso/boot/kernel.elf
	mkdir -p iso/modules
	cp programs/program.elf iso/modules/program.elf
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

run: os.iso disk.img
	qemu-system-i386 -cdrom os.iso -m 2048 -serial stdio -d int,cpu_reset -no-reboot -device ahci,id=ahci -drive file=disk.img,id=disk,if=none,format=raw -device ide-hd,drive=disk,bus=ahci.0

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.nasm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	find . -name "*.o" -delete
	find . -name "*.elf" -delete
	find . -name "*.bin" -delete
	rm -f kernel.elf os.iso
	rm -f disk.img
