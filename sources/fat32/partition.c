#include "partition.h"

void print_part(partition_table_entry_t *part) {
    kprintf("Partition description : \n \
            boot_indicator : %d\n \
            starting_head : %d\n \
            starting_sector : %d\n \
            starting_cylinder : %d\n \
            system_id : %d\n \
            ending_head : %d\n \
            ending_sector : %d\n \
            ending_cylinder : %d\n \
            relative_sector : %d\n \
            nb_sectors : %d\n",
            part->boot_indicator,
            part->starting_head,
            part->starting_sector,
            part->starting_cylinder,
            part->system_id,
            part->ending_head,
            part->ending_sector,
            part->ending_cylinder,
            part->relative_sector,
            part->nb_sectors);
}

void load_partition(int show) {
    read_address(PARTITION_TABLE, sizeof(partition_table_entry_t), &partition);
    kprintf("Partition loaded.\n");
    if (show)
        print_part(&partition);
}