#include "paging.h"

void init_paging() {
    return;
}

void switch_page_directory(page_directory_t *new);
page_t *get_page(u32 address, int make, page_directory_t *dir);
//void page_fault(registers_t regs);
