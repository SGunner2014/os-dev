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

void *elf_load_file(void *file)
{
    Elf32_Ehdr *hdr = (Elf32_Ehdr*) file;

    if (!check_elf_header(hdr)) {
        return NULL;
    }

    switch(hdr->e_type) {
        case ET_EXEC:
            break;
        case ET_REL:
            break;
        default:
            break;
    }

    return NULL;
}
