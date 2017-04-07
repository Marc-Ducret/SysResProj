#ifndef PARTITION_H
#define PARTITION_H
#include "disk.h"
#include "int.h"
#include "printing.h"

#define PARTITION_TABLE 0x1BE

typedef struct {
    u8 boot_indicator;
    u8 starting_head;
    u8 starting_sector : 6;
    u16 starting_cylinder : 10;
    u8 system_id;
    u8 ending_head;
    u8 ending_sector : 6;
    u16 ending_cylinder : 10;   //
    u32 relative_sector;        // Partition's starting LBA
    u32 nb_sectors;             // Total number of sectors
} __attribute__ ((packed)) partition_table_entry_t;

partition_table_entry_t partition;
void load_partition(int show);
#endif /* PARTITION_H */

