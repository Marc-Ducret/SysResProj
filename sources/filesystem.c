#include "filesystem.h"

mbr_t mbr;
fs_t fs;
extern partition_table_entry_t partition;
ft_entry_t file_table[MAX_NB_FILE];
fd_t next_free_fd = 0;

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
}

void get_long_name(long_file_name_t* lfn, char *buffer) {
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

void set_long_name(long_file_name_t* lfn, char *buffer) {
    // TODO Manage the two bytes characters !
    for(int i = 0; i < 5 ; ++i){
        *(((char*) lfn->first5) + 2*i) = buffer[i];
    }
    for(int i = 0 ; i < 6 ; ++i){
        *(((char*) lfn->next6) + 2*i) = buffer[i+5];
    }
    for(int i = 0 ; i < 2; ++i){
        *(((char*) lfn->final2) + 2*i) = buffer[i+11];
    }
}

void get_short_name(directory_entry_t *dirent, char *buffer) {
    // TODO Do it more properly.
    memcpy(buffer, dirent->file_name, 11);
    buffer[11] = 0;
    for (u32 i = 10; buffer[i] == ' '; i--)
        buffer[i] = 0;
}

void read_cluster(u32 cluster, u8* buffer) {
    // Reads the corresponding cluster.
    assert(cluster < fs.nb_data_sectors / fs.sectors_per_cluster);
    // We assume that cluster = 0 means root_directory
    if (!cluster) 
        cluster = fs.root_cluster;
    
    u32 sector = fs.first_data_sector + (cluster - 2) * fs.sectors_per_cluster;
    read_sectors(sector, fs.sectors_per_cluster, (u16*) buffer);
}

void write_cluster(u32 cluster, u8* buffer) {
    // Reads the corresponding cluster.
    assert(cluster < fs.nb_data_sectors / fs.sectors_per_cluster);
    // We assume that cluster = 0 means root_directory
    if (!cluster) {
        kprintf("Warning : use of cluster 0 as root cluster.\n");
        cluster = fs.root_cluster;
    }
    
    u32 sector = fs.first_data_sector + (cluster - 2) * fs.sectors_per_cluster;
    write_sectors(sector, fs.sectors_per_cluster, (u16*) buffer);
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
    return next & 0x0FFFFFFF;
}

void set_next_cluster(u32 cluster, u32 next_cluster) {
    // Sets the FAT entry of cluster to next_cluster
    u32 old_cluster;
    u32 offset = fs.fat_sector * fs.sector_size + 4 * cluster;
    read_address(offset, 4, (u8*) &old_cluster);
    next_cluster &= 0x0FFFFFFF;
    next_cluster |= old_cluster & 0xF0000000;
    write_address(offset, 4, (u8*) &next_cluster);
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

fd_t new_fd() {
    for (fd_t i = next_free_fd; i < MAX_NB_FILE; i++) {
        if (file_table[i].type == F_UNUSED) {
            next_free_fd = i + 1;
            return i;
        }
    }
    for (fd_t i = 0; i < next_free_fd; i++) {
        if (file_table[i].type == F_UNUSED) {
            next_free_fd = i + 1;
            return i;
        }
    }
    
    // No more free entries left.
    assert(0);
    return -1;
}

void free_fd(fd_t fd) {
    file_table[fd].type = F_UNUSED;
}

u32 new_cluster() {
    static u32 next_free_cluster = 0;
    u8 buffer[fs.cluster_size];
    memset(buffer, 0, fs.cluster_size);
    
    for (u32 i = next_free_cluster; i < fs.nb_clusters; i++) {
        if (get_next_cluster(i) == UNUSED_CLUSTER) {
            next_free_cluster = i + 1;
            // Marks the cluster as used (end of chain).
            set_next_cluster(i, END_OF_CHAIN);
            write_cluster(i, buffer);
            return i;
        }
    }
    
    for (u32 i = 0; i < next_free_cluster; i++) {
        if (get_next_cluster(i) == UNUSED_CLUSTER) {
            next_free_cluster = i + 1;
            // Marks the cluster as used (end of chain).
            set_next_cluster(i, END_OF_CHAIN);
            write_cluster(i, buffer);
            return i;
        }
    }
    
    // No more free clusters left
    assert(0);
    return -1;
}

void free_cluster(u32 cluster, u32 prev_cluster) {
    // Removes cluster from the cluster chain without breaking it.
    // If cluster doesn't have any previous cluster, then prev_cluster = 0.
    if (prev_cluster != 0) {
        // We must link prev_cluster to the next one still existing in chain.
        set_next_cluster(prev_cluster, get_next_cluster(cluster));
    }
    // Frees cluster by marking it UNUSED.
    set_next_cluster(cluster, UNUSED_CLUSTER);
}

void free_cluster_chain(u32 cluster) {
    // Removes all the chain starting at cluster.
    u32 next_cluster;
    while (cluster < END_OF_CHAIN) {
        next_cluster = get_next_cluster(cluster);
        free_cluster(cluster, 0);
        cluster = next_cluster;
    }
}

u32 insert_new_cluster(u32 prev, u32 next) {
    // Inserts a new cluster between prev and next clusters.
    u32 new = new_cluster();
    if (new == 0)
        return 0;
    
    set_next_cluster(prev, new);
    set_next_cluster(new, next);
    return new;
}

void reset_cluster(u32 cluster) {
    // Sets this cluster to 0.
    u8 buffer[fs.cluster_size];
    memset(buffer, 0, fs.cluster_size);
    write_cluster(cluster, buffer);
}

void new_entry(u32 cluster, dirent_t *dirent) {
    // Finds size consecutive free entries in cluster chain starting at cluster.
    // Fills in the ent_offset and ent_cluster fields of dirent parameter.
    u8 content[fs.cluster_size];
    read_cluster(cluster, content);
    kprintf("Searching in cluster %d\n", cluster);
    u32 start = 0;
    u32 size = dirent->ent_size;
    for (; content[start] != END_OF_ENTRIES && content[start] != UNUSED_ENTRY;
         start += 32); 
    
    while (start <= fs.cluster_size - size * 32) {
        // Finds the first used entry after start.
        u32 end = start;
        while (content[end] == END_OF_ENTRIES || content[end] == UNUSED_ENTRY) {
            end += 32;
            kprintf("0");
            if (end - start >= size * 32) {
                // Enough place !
                dirent->ent_offset = start;
                dirent->ent_cluster = cluster;
                return;
            }
        }
        start = end + 32; 
        kprintf("1");
    }
    kprintf("Not enough place in cluster %d\n", cluster);
    // Not enough place in this cluster. We have to move to next one.
    u32 next_cluster = get_next_cluster(cluster);
    kprintf("Hello choice : next cluster %d\n", next_cluster);
    if (next_cluster < END_OF_CHAIN) {
        kprintf("Going to cluster %x\n", next_cluster);
        return new_entry(next_cluster, dirent);
    }
    
    // No more cluster in the chain. Must create a new one, link it, and fill
    // in the gap with unused entries.
    while (start < fs.cluster_size) {
        if (content[start] == END_OF_ENTRIES)
            content[start] = UNUSED_ENTRY;
        start++;
    }
    write_cluster(cluster, content);
    next_cluster = new_cluster();
    kprintf("Just created cluster %d\n", next_cluster);
    set_next_cluster(cluster, next_cluster);
    kprintf("Next cluster 1 : %d, and 2 : %d\n", get_next_cluster(cluster), get_next_cluster(next_cluster));
    dirent->ent_cluster = next_cluster;
    dirent->ent_offset = 0;
}

void free_entry(dirent_t *dirent) {
    u32 offset = dirent->ent_offset;
    u32 cluster = dirent->ent_cluster;
    u32 size = dirent->ent_size;
    u32 prev_cluster = dirent->ent_prev_cluster;
    
    u8 content[fs.cluster_size];
    read_cluster(cluster, content);
    
    // Checks if this entry is the last of its cluster.
    int is_last = 1;
    for (u32 i = offset + size * 32; i < fs.cluster_size; i += 32) {
        if (content[i] == END_OF_ENTRIES)
            break;
        if (content[i] != UNUSED_ENTRY) {
            is_last = 0;
            break;
        }
    }
    
    int is_first = 1;
    for (u32 i = 0; i < offset; i += 32) {
        if (content[i] != UNUSED_ENTRY) {
            is_first = 0;
            break;
        }
    }
    
    if (is_first && is_last) {
        free_cluster(cluster, prev_cluster);
        return;
    }
    else if (get_next_cluster(cluster) == END_OF_CHAIN) {
        // We have to extend the END_OF_ENTRIES
        for (u32 i = offset + (size - 1) * 32; content[i] == UNUSED_ENTRY; i -= 32) {
            content[i] = END_OF_ENTRIES;
        }
    }
    else {
        // We only need to replace the deleted entries by UNUSED_ENTRY
        for (u32 i = 0; i < size; i++)
            content[offset + 32 * i] = UNUSED_ENTRY;
    }
    
    // Writes the changes on the disk.
    write_cluster(cluster, content);
}