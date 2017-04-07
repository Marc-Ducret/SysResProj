#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "int.h"
#include "disk.h"
#include "printing.h"
#include "partition.h"
#include "lib.h"
#include "stddef.h"

typedef struct {
    u8 start_code[3];
    char oem_ident[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    u8 nb_fat;
    u16 nb_dirent;
    u16 nb_sectors;
    u8 media_desc_type;
    u16 unused;   // Not used in FAT 32
    u16 sectors_per_track; // Not really reliable
    u16 nb_heads;          // Not really reliable
    u32 hidden_sectors;
    u32 nb_sectors_large;
    
    // Extended Boot Record
    u32	sectors_per_fat;    // Size of FAT in sectors.
    u16	extended_flags;
    u16	fat_version;
    u32	root_cluster;       // Cluster number of root directory.       
    u16	fat_info_sector;    // Sector number of FS info structure.
    u16	backup_sector;   
    u8 reserved_0[12];      // Should be zero.
    u8 drive_number;        // 0x00 for floppy, 0x80 for hard disks.
    u8 reserved_1;
    u8 boot_signature;      // Must be 0x28 ot 0x29
    u32 volume_id;          // Serial Number
    char volume_label[11];
    char fat_type_label[8]; // Not reliable, always 'FAT32   '
} __attribute__ ((packed)) mbr_t;

typedef struct {
    u32 hour    : 5;
    u32 minutes : 6;
    u32 seconds : 5;
} __attribute__ ((packed)) time_t;

typedef struct {
    u32 year  : 7;
    u32 month : 4;
    u32 day   : 5;
} __attribute__ ((packed)) date_t;

typedef struct {
    u32 rd_only : 1;
    u32 hidden  : 1;
    u32 system  : 1;
    u32 vol_id  : 1;
    u32 dir     : 1;
    u32 archive : 1;
    u32 unused  : 2;
} __attribute__ ((packed)) attributes_t;

typedef struct {
    char file_name[11];
    attributes_t attributes;    // Attributes
    u8 reserved;
    u8 creation_duration;       // In tenth of seconds
    time_t creation_time;       // Time the file was created
    date_t creation_date;       // Date the file was created
    date_t access_date;            // Last accessed date
    u16 cluster_high;           // 16 high bits of first cluster number
    time_t modification_time;   // Last modificaton time
    date_t modification_date;   // Last modification date
    u16 cluster_low;            // 16 low bits of first cluster number
    u32 file_size;              // Size of the file in bytes
} __attribute__ ((packed)) directory_entry_t;

typedef struct {
    u8 order;           // Order of this entry in sequence 
                        //  of long file name entries
    u16 first5[5];      // First 5 2-bytes characters.
    u8 attribute;       // Always 0x0F for long file name.
    u8 long_entry_type; // Always 0 for name entries
    u8 checksum;
    u16 next6[6];       // Next 6 2-bytes characters.
    u16 zero;           // Always zero
    u16 final2[2];      // Final 2 2-bytes characters.
} __attribute__ ((packed)) long_file_name_t;

typedef struct {
    u32 start_sector;       // All sectors are absolute, clusters are relative
    u32 nb_sectors;
    u32 end_sector;
    u32 backup_sector;
    u32 info_sector;
    u32 first_data_sector;
    u32 fat_sector;
    u32 sector_size;
    u32 nb_data_sectors;
    u32 sectors_per_cluster;
    u32 reserved_sectors;
    u32 sectors_per_fat;
    u32 nb_fat;
    u32 root_cluster;
    u32 nb_clusters;
    u32 cluster_size;
} fs_t;

void init_fs(int show);
void load_mbr(int show);
void read_cluster(u32 cluster, u8* buffer);
void read_dir_cluster(u32 cluster);
void get_name(long_file_name_t* lfn, char *buffer);
u32 get_next_cluster(u32 cluster);
u32 get_cluster(directory_entry_t *dirent);
#endif /* FILESYSTEM_H */

