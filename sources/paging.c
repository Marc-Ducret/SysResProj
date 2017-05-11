#include "paging.h"
#include "printing.h"
#include "lib.h"
#include "memory.h"
#include <stddef.h>
#include "kernel.h"
#include "fs_call.h"

// Frames table
u32 memory_end; // Size of the memory covered by pages
u32 nb_frames; // Maximum number of frames
u32 size_frames_table; // Size of the frames table
u32 *frames; // Pointer to the frames table

//Page directory
page_directory_t *identity_pd = NULL;
page_directory_t *current_page_directory = NULL;

void set_frame(u32 frame_addr) {
    // Sets the corresponding frame as used.
    u32 frame = frame_addr / PAGE_SIZE;
    u32 table_index = frame >> 5;
    u32 table_offset = frame & 0x1F;
    frames[table_index] |= 1 << table_offset;
}

void clear_frame(u32 frame_addr) {
    // Sets the corresponding frame as unused.
    u32 frame = frame_addr / PAGE_SIZE;
    u32 table_index = frame >> 5;
    u32 table_offset = frame & 0x1F;
    frames[table_index] &= ~(1 << table_offset);

}

u32 test_frame(u32 frame_addr) {
    // Returns 0 if this frame isn't used, 1 otherwise.
    u32 frame = frame_addr / PAGE_SIZE;
    u32 table_index = frame >> 5;
    u32 table_offset = frame & 0x1F;
    return (frames[table_index] >> table_offset) & 1;
}

u32 new_frame() {
    // Finds the first unused frame in memory.
    u32 index, offset;
    for (index = 0x0; index < size_frames_table; index++) {
        if (~frames[index]) {
            // This index contains a free frame.
            u32 value = frames[index];
            for (offset = 0; offset < 32; offset++) {
                if (!(value & 1)) {
                    return (index << 5) + offset;
                }
                value = value >> 1;
            }
        }
    }
    return nb_frames;
}

void map_page(page_t* page, u32 phys_address, int is_kernel, int is_writable) {
    // Maps the specified page to the frame at phys_address.
    // Uses the parameters to determine the flags.
    // Allocates a frame for this page, if not already done.
    
    
    if (page->present) {
        asm("cli");
        kprintf("Mapping present page %x to %x\n", page, phys_address);
        asm("hlt");
    }
    
    u32 frame = phys_address ? phys_address >> 12 : new_frame();
    
    if (frame == nb_frames) {
        kprintf("No more frames left. Not good.\n");
        asm("hlt");
        return;
    }

    set_frame(frame << 12);
    page->present = 1;
    page->frame = frame;
    page->rw = is_writable? 1:0;
    page->user = is_kernel? 0:1;
    page->accessed = 0;
    page->dirty = 0;
}

void alloc_page(page_t *page, int is_kernel, int is_writable) {
    // Allocates a frame for this page, if not already done.
    map_page(page, 0, is_kernel, is_writable);
}

void invalidate(u32 address) {
    asm volatile("invlpg (%0)" ::"r" (address) : "memory");
}

void free_page(page_t * page, u32 address) {
    // Frees this page, if not already done.
    
    if (!page->present)
        return;
    
    clear_frame(page->frame << 12);
    page->present = 0;
    page->frame = 0;
    invalidate(address);
}

page_t *get_page(u32 address, int make, page_directory_t* directory) {
    // Gets a pointer to the page entry.
    // If make, creates the corresponding page table if it doesn't exist.
    address = address >> 12;      // Index of the page
    u32 index = address >> 10;    // Index in page_directory
    u32 offset = address & 0x3FF; // Index in page_table

    if (directory->tables[index]) {
        // The page table already exists.
        return &(directory->tables[index]->pages[offset]);
    } else if (make) {
        // Creates the corresponding page table.
        u32 tmp;
        page_table_t* new_table = (page_table_t*) 
                kmalloc_3(sizeof(page_table_t), 1, &tmp);
        memset(new_table, 0, sizeof(page_table_t));
        
        directory->tables[index] = new_table;
        directory->tablesPhysical[index].table_addr = tmp >> 12;
        directory->tablesPhysical[index].present = 1;
        directory->tablesPhysical[index].accessed = 0;
        directory->tablesPhysical[index].rw = 1;
        directory->tablesPhysical[index].user = 0; //TODO ?
        
        return &(new_table->pages[offset]);
    } else {
        return NULL;
    }
}

void switch_page_directory(page_directory_t* directory) {
    current_page_directory = directory;
    asm volatile("mov %0, %%cr3":: "r"(&directory->tablesPhysical));
    u32 cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void switch_to_default_page_directory() {
    switch_page_directory(identity_pd);
}

page_directory_t *get_identity() { return identity_pd; }

void init_paging(u32 mem_end) {
    // Init of frames table
    memory_end = mem_end;
    nb_frames = memory_end >> 12;
    size_frames_table = nb_frames >> 5;
    frames = (u32*) kmalloc(size_frames_table << 2);
    memset((u8*) frames, 0, size_frames_table << 2);
    
    // Init of page directory
    identity_pd = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));
    memset((u8*)identity_pd, 0, sizeof(page_directory_t));

    // Identity paging
    // Allocates only what we need
    kprintf("kernel mem end %x\n", kernel_mem_end);
    for(u32 i = 0; i < mem_end; i += PAGE_SIZE) {
        page_t *page = get_page(i, 1, identity_pd);
        if(i < kernel_mem_end) map_page(page, i+1, 1, 1);
    }
    switch_page_directory(identity_pd);
    kprintf("Paging initialized.\n");
    return;
}

u32 tmp;

int copy_bin(u32 buffer_code_addr, u32 user_code_len, page_directory_t *user_pd, page_directory_t *cur_pd) {
    // Remaps the user_code_len bytes at buffer_code_address to their good location 
    // in user_pd.
    void *phys_addr;
    page_t *page;
    for (u32 i = 0; i < CODE_LEN; i += PAGE_SIZE) {
        page = get_page(buffer_code_addr + i, 0, cur_pd);
        phys_addr = get_physical(page);
        if (1 || i < user_code_len) { // TODO ELF -> data segment !
            // Maps the physical memory in the new page directory.
            map_page(get_page(USER_CODE_VIRTUAL + i, 1, user_pd), (u32) phys_addr, 0, 1);
        }
        page->present = 0;
        page->frame = 0;
        invalidate(buffer_code_addr + i);
    }
    return 0;
}

page_directory_t *init_user_page_dir(fd_t fd, char *args, page_directory_t *cur_pd) {
    asm volatile ("mov %cr3, %eax   \n"
                  "mov %eax, tmp ");
    cur_pd = (page_directory_t*) (tmp - 0x1000); //TODO do better
    
    // Creates a new page directory and initializes it with specified binary.
    void *user_code = (void *) USER_CODE_VIRTUAL - CODE_LEN;
    // TODO Do it with only one page ?
    page_t *first_page = get_page((u32) user_code, 0, cur_pd);
    if (first_page != NULL && first_page->present) {
        // The page already exists. No more memory.
        errno = ENOMEM;
        return NULL;
    }
    
    page_directory_t *pd = kmalloc_a(sizeof(page_directory_t)); // TODO Real malloc ?
    if (pd == NULL) {
        int err = errno;
        close(fd);
        errno = err;
        return NULL;
    }
    memset(pd, 0, sizeof(page_directory_t));
    
    first_page = get_page((u32) user_code, 1, cur_pd);
    map_page(first_page, 0, 1, 1);
    char *dst = user_code;
    while((*(dst++) = *(args++)));
    map_page(get_page(USER_ARGS_BUFFER, 1, pd), (u32) get_physical(first_page), 0, 1);
    first_page->present = 0;
    first_page->frame = 0;
    invalidate((u32) user_code);
    
    // TODO only maps needed pages ?
    for (u32 i = 0; i < CODE_LEN; i += PAGE_SIZE) {
        map_page(get_page((u32) user_code + i, 1, cur_pd), 0, 1, 1);
    }
    ssize_t len = read(fd, user_code, CODE_LEN);

    if (len == -1) {
        int err = errno;
        close(fd);
        errno = err;
        return NULL;
    }
    close(fd);
    copy_bin((u32) user_code, len, pd, cur_pd);
    
    // The binary is loaded, now we complete the kernel part, and others.
    for(u32 i = 0; i < kernel_mem_end; i += PAGE_SIZE)
        map_page(get_page(i, 1, pd), i+1, 1, 1);
    map_page(get_page(USER_STACK_VIRTUAL, 1, pd), 0, 0, 1); //STACK
    map_page(get_page(USER_SCREEN_VIRTUAL, 1, pd), 0, 0, 1); //SCREEN
    return pd;
}

void *get_physical(page_t *page) {
    return (void*) (page->frame << 12);
}

u8 page_fault(context_t* context) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    u32 faulting_address;

    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us details of what happened.
    int err_code = context->err_code;
    int present  = err_code & 0x1;     // Page present
    int rw = err_code & 0x2;           // Write operation?
    int us = err_code & 0x4;           // Processor was in user-mode?
    int reserved = err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = err_code & 0x10;          // Caused by an instruction fetch?
    
    // Output an error message.
    kprintf("Page fault! ( ");
    if (present) 
        kprintf("present ");
    if (!rw)
        kprintf("read ");
    else
        kprintf("write ");
    if (us)
        kprintf("user-mode ");
    if (reserved) 
        kprintf("reserved ");
    kprintf(") id=%d at %x (eip = %x pid = %d)\n", id, faulting_address, 
                            context->stack.eip, get_global_state()->curr_pid);
    asm("cli");
    asm("hlt");
    return 1;
}

int check_address(void *address, int user, int write, page_directory_t *pd) {
    // Checks the permissions of given address in the page directory.
    page_t *page = get_page((u32) address, 0, pd);
    if (page == NULL || !page->present) {
        errno = EFAULT;
        return -1;
    }
    if ((!page->user && user) || (!page->rw && write)) {
        errno = EFAULT;
        kprintf("Voila : address %x, page_user %d, page_write %d, user %d, write %d\n",
                address, page->user, page->rw, user, write);
        asm("hlt");
        return -1;
    }
    return 0;
}
