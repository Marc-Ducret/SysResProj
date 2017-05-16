#ifndef PAGING_H
#define PAGING_H

#include "int.h"
#include "context.h"


#define PAGE_SIZE 0x1000
#define CODE_LEN 0x30000
#define USER_CODE_VIRTUAL       0x40000000
#define USER_STACK_VIRTUAL      0x80000000
#define USER_STACK_SIZE         0x00010000
#define USER_SCREEN_VIRTUAL     0x88000000
#define USER_ARGS_BUFFER        0x88001000
#define USER_HEAP               0x88002000

typedef struct pde
{
    u32 present    : 1;   // Page present in memory
    u32 rw         : 1;   // Read-only if clear, readwrite if set
    u32 user       : 1;   // Supervisor level only if clear
    u32 pwt        : 1;   // Page-level write through
    u32 pcd        : 1;   // Page-level cache disable
    u32 accessed   : 1;   // Has the page been accessed since last refresh?
    u32 unused     : 6;   // Amalgamation of unused and reserved bits
    u32 table_addr : 20;  // Table address (shifted right 12 bits)
} pde_t;

typedef struct page
{
    u32 present    : 1;   // Page present in memory
    u32 rw         : 1;   // Read-only if clear, readwrite if set
    u32 user       : 1;   // Supervisor level only if clear
    u32 pwt        : 1;   // Page level write through
    u32 pcd        : 1;   // Page-level cache disable
    u32 accessed   : 1;   // Has the page been accessed since last refresh?
    u32 dirty      : 1;   // Has the page been written to since last refresh?
    u32 unused     : 5;   // Amalgamation of unused and reserved bits
    u32 frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
    page_t pages[1024];
} page_table_t;

typedef struct page_directory
{

    //Array of page directory entries.
    page_table_t *tables[1024];
    
    //Array of pointers to the pagetables above, but gives their *physical*
    //location, for loading into the CR3 register.
    pde_t tablesPhysical[1024];
    
    //The physical address of tablesPhysical. This comes into play
    //when we get our kernel heap allocated and the directory
    //may be in a different location in virtual memory.
    u32 physicalAddr;
} page_directory_t;
#include "filesystem.h"
void init_paging();
page_directory_t *init_user_page_dir(fd_t fd, char *args, page_directory_t *cur_pd);
void switch_page_directory(page_directory_t *directory);
void switch_to_default_page_directory();
page_t *get_page(u32 address, int make, page_directory_t *dir);
void *get_physical(page_t *page);
u8 page_fault(context_t* context);
page_directory_t *get_identity();
void map_page(page_t *page, u32 phys_address, int is_kernel, int is_writable);
void free_page(page_t *page, u32 address);
int check_address(void *address, int user, int write, page_directory_t *pd);
void free_page_directory(page_directory_t *pd);

#endif /* PAGING_H */

