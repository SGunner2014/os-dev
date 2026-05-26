#include "elf.h"

bool check_elf_header(Elf32_Ehdr *hdr)
{
    if (!hdr)
        return false;

    if (
        hdr->e_ident[EI_MAG0] != ELFMAG0 ||
        hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 ||
        hdr->e_ident[EI_MAG3] != ELFMAG3
    ) {
        return false;
    }

    return true;
}

static Elf32_Phdr *elf_load_phdr(Elf32_Ehdr *hdr, uint32_t num)
{
    // Get the original pointer to the elf file, and convert to byte ptr
    uint8_t *ptr = (uint8_t*) hdr;
    ptr += hdr->e_phoff;
    // Add program header offset in bytes
    Elf32_Phdr *phdr = (Elf32_Phdr*) ptr;
    phdr += num;
    // Then cast back to phdr* and use num as offset
    return phdr;
}

static void *elf_load_p_section(
    Elf32_Ehdr *hdr,
    uint32_t vaddr,
    uint32_t p_size,
    uint32_t m_size,
    uint32_t offset
)
{

}

static void *elf_load_exec(Elf32_Ehdr *hdr)
{
    for (uint32_t i = 0; i < hdr->e_phnum + 1; i++) {
        Elf32_Phdr *p_hdr = elf_load_phdr(hdr, i);

        // Only load sections that we need to
        if (p_hdr->p_type == ELF_PT_LOAD) {
            uint32_t vaddr = p_hdr->p_vaddr;
            uint32_t p_size = p_hdr->p_filesz;
            uint32_t m_size = p_hdr->p_memsz;
            uint32_t offset = p_hdr->p_offset;
        }
    }
}

void *elf_load_file(void *file)
{
    Elf32_Ehdr *hdr = (Elf32_Ehdr*) file;

    if (!check_elf_header(hdr)) {
        return NULL;
    }

    switch(hdr->e_type) {
        case ET_EXEC:
            return elf_load_exec(hdr);
        case ET_REL:
            break;
        default:
            break;
    }

    return NULL;
}
