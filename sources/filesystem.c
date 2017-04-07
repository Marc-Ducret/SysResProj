#include "filesystem.h"

mbr_t mbr;
fs_t fs;
extern partition_table_entry_t partition;

void print_mbr(mbr_t *mbr) {
    kprintf("MBR description : \n \
            OEM Ident : %s\n \
            Bytes_per_sector : %d \n \
            Sectors_per_cluster : %d \n \
            Reserved_sectors : %d \n \
            Nb_fat : %d \n \
            Nb_dirent : %d \n \
            Nb_sectors : %d \n \
            media_desc_type : %d \n \
            sectors_per_fat : %d \n \
            sectors_per_track : %d \n \
            nb_heads : %d \n \
            hidden_sectors : %d \n \
            nb_sectors_large : %d \n \
            sectors_per_fat : %d \n \
            extended_flags : %d \n \
            fat_version : %d \n \
            root_cluster : %d \n \
            fat_info_sector : %d \n \
            backup_sector : %d \n \
            drive_number : %d \n \
            boot_signature : %d \n \
            volume_id : %d \n \
            volume_label : %s \n \
            fat_type_label : %s \n\n",
            mbr->oem_ident,
            mbr->bytes_per_sector,
            mbr->sectors_per_cluster,
            mbr->reserved_sectors,
            mbr->nb_fat,
            mbr->nb_dirent,
            mbr->nb_sectors,
            mbr->media_desc_type,
            mbr->sectors_per_fat,
            mbr->sectors_per_track,
            mbr->nb_heads,
            mbr->hidden_sectors,
            mbr->nb_sectors_large,
            mbr->sectors_per_fat,
            mbr->extended_flags,
            mbr->fat_version,
            mbr->root_cluster,       
            mbr->fat_info_sector,
            mbr->backup_sector,
            mbr->drive_number,
            mbr->boot_signature,
            mbr->volume_id,
            mbr->volume_label,
            mbr->fat_type_label);
}

void print_fs() {
    kprintf("File System Description : \n \
            start_sector : %d \n \
            nb_sectors : %d \n \
            end_sector : %d \n \
            backup_sector : %d \n \
            info_sector : %d \n \
            first_data_sector : %d \n \
            fat_sector : %d \n \
            sector_size : %d \n \
            nb_data_sectors : %d \n \
            sectors_per_cluster : %d \n \
            reserved_sectors : %d \n \
            sectors_per_fat : %d \n \
            nb_fat : %d \n \
            root_cluster : %d \n \
            nb_clusters : %d \n \
            cluster_size : %d \n\n",
            fs.start_sector,
            fs.nb_sectors,
            fs.end_sector,
            fs.backup_sector,
            fs.info_sector,
            fs.first_data_sector,
            fs.fat_sector,
            fs.sector_size,
            fs.nb_data_sectors,
            fs.sectors_per_cluster,
            fs.reserved_sectors,
            fs.sectors_per_fat,
            fs.nb_fat,
            fs.root_cluster,
            fs.nb_clusters,
            fs.cluster_size);
}

void load_mbr(int show) {
    read_address(partition.relative_sector * 512, sizeof(mbr_t), &mbr);
    kprintf("MBR loaded.\n");
    if (show)
        print_mbr(&mbr);
}

void init_fs(int show) {
    load_partition(show);
    load_mbr(show);
    fs.start_sector = partition.relative_sector;
    assert(partition.nb_sectors == (mbr.nb_sectors ?
                                    mbr.nb_sectors :
                                    mbr.nb_sectors_large));
    fs.nb_sectors = partition.nb_sectors;
    fs.end_sector = fs.start_sector + fs.nb_sectors - 1;
    fs.reserved_sectors = mbr.reserved_sectors;
    fs.sectors_per_cluster = mbr.sectors_per_cluster;
    fs.sector_size = mbr.bytes_per_sector;
    fs.fat_sector = fs.start_sector + fs.reserved_sectors;
    fs.backup_sector = fs.start_sector + mbr.backup_sector;
    fs.info_sector = fs.start_sector + mbr.fat_info_sector;
    fs.sectors_per_fat = mbr.sectors_per_fat;
    fs.nb_fat = mbr.nb_fat;
    fs.nb_data_sectors = fs.nb_sectors - fs.reserved_sectors 
                         - fs.nb_fat * fs.sectors_per_fat;
    fs.first_data_sector = fs.end_sector - fs.nb_data_sectors + 1;
    fs.root_cluster = mbr.root_cluster;
    fs.cluster_size = fs.sectors_per_cluster * fs.sector_size;
    fs.nb_clusters = fs.nb_data_sectors / fs.sectors_per_cluster;
    
    kprintf("File System initialised.\n");
    if (show)
        print_fs();
        kprintf("Content of root directory : \n");
        read_dir_cluster(fs.root_cluster);
        kprintf("Content of boot directory : \n");
        read_dir_cluster(3);
        kprintf("Content of grub directory : \n");
        read_dir_cluster(4);    
}

void get_name(long_file_name_t* lfn, char *buffer) {
    // TODO Manage the two bytes characters !
    for(int i = 0; i < 5 ; ++i){
        buffer[i] = *(((char*) lfn->first5) + 2*i);
    }
    for(int i = 0 ; i < 6 ; ++i){
        buffer[i+5] = *(((char*) lfn->next6) + 2*i);
    }
    for(int i = 0 ; i < 2; ++i){
        buffer[i+11] = *(((char*) lfn->final2) + 2*i);
    }
}

void read_cluster(u32 cluster, u8* buffer) {
    // Reads the corresponding cluster.
    
    // We assume that cluster = 0 means root_directory
    if (!cluster) 
        cluster = fs.root_cluster;
    
    u32 sector = fs.first_data_sector + (cluster - 2) * fs.sectors_per_cluster;
    read_sectors(sector, fs.sectors_per_cluster, (u16*) buffer);
}

u32 get_next_cluster(u32 cluster) {
    // If we want to avoid redundant readings in FAT table
    // This reads the whole sector and could save it temporarily.
    // u32 sector = (cluster * 4) / fs.sector_size + fs.fat_sector;
    // u32 offset = (cluster * 4) % fs.sector_size;
    // u32 buffer[128];
    // read_sectors(sector, 1, (u8*) buffer);
    // return buffer[offset];
    u32 next;
    read_address(fs.fat_sector * fs.sector_size + 4 * cluster, 4, (u8*) &next);
    return next;
}

u32 get_cluster(directory_entry_t *dirent) {
    return (( dirent->cluster_high) << 16) | (dirent->cluster_low);
}

void print_dirent(directory_entry_t *dirent, char *buffer) {
    // Prints this directory entry with long name in buffer.
    // If buffer is NULL, then uses normal name.
    char tmp_buffer[12];
    if (buffer == NULL) {
        // Normal name.
        for (int i = 0; i < 11; i++) {
            tmp_buffer[i] = dirent->file_name[i];
        }
        tmp_buffer[11] = 0x0;
        buffer = tmp_buffer;
    }
    attributes_t *attr = &(dirent->attributes);
    kprintf(buffer);
    kprintf(" (");
    if (attr->dir) {
        kprintf("Directory");
    }
    else {
        kprintf("File");
    }
    if (attr->rd_only) {
        kprintf(", RO");
    }
    else {
        kprintf(", RW");
    }
    kprintf(", Size %d ko", dirent->file_size / 1024 + 
                            (dirent->file_size % 1024 ? 1 : 0));
    kprintf(", Cluster %d", get_cluster(dirent));
    
    kprintf(")\n");   
}

void read_dir_cluster(u32 cluster) {
    // Reads and prints every entry of the corresponding directory.
    u8 buffer[fs.cluster_size];
    u32 next_cluster = 0;
    // Create a buffer to store long names.
    char tmp_buffer[30];
    char *curr_index;
    curr_index = tmp_buffer;

    do {
        read_cluster(cluster, buffer);

        for (u32 i = 0; i < fs.cluster_size; i += 32) {
            if (buffer[i] == 0) {
                // No more directories or files.
                break;
            }
            if (buffer[i] == 0xE5) {
                // Unused entry
                continue;
            }
            if (buffer[i + 11] == 0x0F) {
                // Long name entry TODO Create a specific function  with loops ?
                long_file_name_t *lfn = (long_file_name_t *) (buffer+i);
                get_name(lfn, curr_index);
                curr_index += 13;
                continue;
            }            
            else {
                // This is a directory entry.
                directory_entry_t *dirent = (directory_entry_t *) (buffer+i);
                *(curr_index + 1) = 0;
                print_dirent(dirent, (tmp_buffer == curr_index) ? NULL : tmp_buffer);
                curr_index = tmp_buffer;
            }
        }
        next_cluster = get_next_cluster(cluster);
    } while (next_cluster < 0x0FFFFFF8);
    kprintf("\n");
}
