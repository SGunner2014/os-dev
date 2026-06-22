#include "elf.h"
#include "../cpu/mem.h"
#include "../drivers/vga.h"
#include "utils.h"

bool check_elf_header(Elf32_Ehdr *hdr)
{
    if (!hdr)
        return false;

    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 || hdr->e_ident[EI_MAG3] != ELFMAG3) {
        return false;
    }

    return true;
}

static Elf32_Phdr *elf_load_phdr(Elf32_Ehdr *hdr, uint32_t num)
{
    // Get the original pointer to the elf file, and convert to byte ptr
    uint8_t *ptr = (uint8_t *)hdr;
    ptr += hdr->e_phoff;
    // Add program header offset in bytes
    Elf32_Phdr *phdr = (Elf32_Phdr *)ptr;
    phdr += num;
    // Then cast back to phdr* and use num as offset
    return phdr;
}

static void elf_load_p_section(Elf32_Ehdr *hdr, uint32_t vaddr, uint32_t m_size,
                               uint32_t offset, Process *process)
{
    uint8_t *hdr_ptr = (uint8_t *)hdr;
    hdr_ptr += offset;
    uint32_t *ptr = (uint32_t *)((uint32_t)hdr_ptr - PHYS_OFFSET);

    print("Mapping physical: ");
    char buff[64];
    itoa((uint32_t)ptr, buff, 16);
    print(buff);
    print("\n");

    // We need to go through process and map the virt to phys for the block
    uint32_t *a_vaddr = get_page_aligned((uint32_t *)vaddr);
    uint32_t *a_paddr = get_page_aligned(ptr);

    map_virt_range(process->page_directory_virt, process->virtual_pde, a_vaddr,
                   a_paddr, m_size);

    // map_custom_virt_range(process, ptr, (uint32_t*) vaddr, m_size);
    print("Mapped custom virt range\n");
}

static Process *elf_load_exec(Elf32_Ehdr *hdr)
{
    // Initialise a new empty process
    print("Creating process\n");
    Process *process = create_empty_process();

    uint32_t max_vaddr = 0;

    for (uint32_t i = 0; i < (uint32_t)hdr->e_phnum + 1; i++) {
        print("Loading phdr\n");
        Elf32_Phdr *p_hdr = elf_load_phdr(hdr, i);

        // Only load sections that we need to
        if (p_hdr->p_type == ELF_PT_LOAD) {
            uint32_t vaddr = p_hdr->p_vaddr;
            // uint32_t p_size = p_hdr->p_filesz;
            uint32_t m_size = p_hdr->p_memsz;
            uint32_t offset = p_hdr->p_offset;

            uint32_t vaddr_end = vaddr + m_size;
            if (vaddr_end > max_vaddr) {
                max_vaddr = vaddr_end;
            }

            elf_load_p_section(hdr, vaddr, m_size, offset, process);
        }
    }

    process->brk = process->start_brk =
        (uint32_t *)((max_vaddr / PAGE_SIZE + 1) * PAGE_SIZE);

    print("Loaded all sections\n");

    process->prog = (call_module_t)hdr->e_entry;

    return process;
}

Process *elf_load_file(uint32_t *file)
{
    Elf32_Ehdr *hdr = (Elf32_Ehdr *)file;

    if (!check_elf_header(hdr)) {
        return NULL;
    }

    switch (hdr->e_type) {
    case ET_EXEC:
        return elf_load_exec(hdr);
    case ET_REL:
        break;
    default:
        break;
    }

    return NULL;
}
