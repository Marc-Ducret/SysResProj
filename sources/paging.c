#include "paging.h"
#include "printing.h"
#include "lib.h"
#include "memory.h"
#include <stddef.h>

// Frames table
u32 memory_end; // Size of the meory covered by pages
u32 nb_frames; // Maximum number of frames
u32 size_frames_table; // Size of the frames table
u32 *frames; // Pointer to the frames table

//Page directory
page_directory_t *page_directory;
page_directory_t *current_page_directory;

void set_frame(u32 frame_addr) {
    // Sets the corresponding frame as used.
    u32 frame = frame_addr / 0x1000;
    u32 table_index = frame >> 5;
    u32 table_offset = frame & 0x1F;
    frames[table_index] |= 1 << table_offset;
}

void clear_frame(u32 frame_addr) {
    // Sets the corresponding frame as unused.
    u32 frame = frame_addr / 0x1000;
    u32 table_index = frame >> 5;
    u32 table_offset = frame & 0x1F;
    frames[table_index] &= ~(1 << table_offset);

}

u32 test_frame(u32 frame_addr) {
    // Returns 0 if this frame isn't used, 1 otherwise.
    u32 frame = frame_addr / 0x1000;
    u32 table_index = frame >> 5;
    u32 table_offset = frame & 0x1F;
    return (frames[table_index] >> table_offset) & 1;
}

u32 new_frame() {
    // Finds the first unused frame in memory.
    u32 index, offset;
    for (index = 0; index < size_frames_table; index++) {
        if (~frames[index]) {
            // This index contains a free frame.
            u32 value = frames[index];
            for (offset = 0; offset < 32; offset++) {
                if (!(value & 1)) {
                    return index << 5 + offset;
                }
                value >> 1;
            }
        }
    }
    return nb_frames;
}

void alloc_page(page_t *page, int is_kernel, int is_writable) {
    // Allocates a frame for this page, if not already done.
    if (page->present)
        return;
    
    u32 frame = new_frame();
    
    if (frame == nb_frames) {
        kprintf("No more frames left. Not good.\n");
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

void free_page(page_t * page) {
    // Frees this page, if not already done.
    
    if (!page->present)
        return;
    
    clear_frame(page->frame);
    page->present = 0;
    page->frame = 0;
}

page_t *get_page(u32 address, int make, page_directory_t* directory) {
    // Gets a pointer to the page entry.
    // If make, creates the corresponding page table if it doesn't exist.
    address = address >> 10;      // Index of the page
    u32 index = address >> 10;    // Index in page_directory
    u32 offset = address & 0x3FF; // Index in page_table
    
    if (directory->tables[index]) {
        // The page table already exists.
        return &(directory->tables[index]->pages[offset]);
    }
    else {
        if (make) {
            // Creates the corresponding page table.
            u32 tmp;
            page_table_t* new_table = (page_table_t*) 
                    kmalloc_3(sizeof(page_table_t), 1, &tmp);
            memset((u8*) new_table, 0, 0x1000);
            
            directory->tables[index] = new_table;
            directory->tablesPhysical[index].table_addr = tmp >> 12;
            directory->tablesPhysical[index].present = 1;
            directory->tablesPhysical[index].accessed = 0;
            directory->tablesPhysical[index].rw = 1;
            directory->tablesPhysical[index].user = 0; //TODO ?
            kprintf("Page table : %d", new_table);
            return &(new_table->pages[offset]);
        }
        else {
            return NULL;
        }
    }
    
}

void switch_page_directory(page_directory_t* directory) {
    current_page_directory = directory;
    asm volatile("mov %0, %%cr3":: "r"(&directory->tablesPhysical));
    BREAKPOINT;
    u32 cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void init_paging(u32 mem_end) {
    // Init of frames table
    memory_end = mem_end;
    nb_frames = memory_end >> 12;
    size_frames_table = nb_frames >> 5;
    frames = (u32*) kmalloc(size_frames_table << 2);
    memset((u8*) frames, 0, size_frames_table << 2);
    
    // Init of page directory
    page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));
    memset((u8*)page_directory, 0, sizeof(page_directory_t));

    // Identity paging
    u32 i = 0;
    
    while (i < free_address) {
        alloc_page(get_page(i, 1, page_directory), 0, 0);
        i += 0x1000;
    }
    kprintf("Suspense..");
    BREAKPOINT;
    switch_page_directory(page_directory);
    BREAKPOINT;
    kprintf("Paging initialized.\n");
    return;
}

//void page_fault(registers_t regs);