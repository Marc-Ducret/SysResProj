#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "int.h"

#include "lib.h"
#include "stddef.h"
#include "stdint.h"

#define END_OF_CHAIN 0x0FFFFFF8
#define UNUSED_CLUSTER 0x0
#define UNUSED_ENTRY 0xE5
#define END_OF_ENTRIES 0x00

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

fs_t fs;

#define MAX_FILE_NAME 64
#define MAX_NB_FILE 256
#define MAX_PATH_NAME 1024
#define O_CRDONLY 1
#define O_CSYSTEM 2
#define O_RDONLY 4
#define O_WRONLY 8
#define O_CREAT  16
#define O_APPEND 32
#define O_TRUNC  64
#define O_RDWR 12
#define O_CMODE 3

typedef u8 oflags_t;

typedef struct {
    int system : 1;
    int rdonly : 1;
} __attribute__ ((packed)) mode_t;

#define RDONLY 1
#define SYSTEM 1

typedef enum {
    F_UNUSED, FILE, DIR
} ftype_t;

typedef struct {
    u32 global_offset; // New (and not next) global offset
    u32 old_offset; // Old (and current) global offset
    oflags_t flags;
    u32 size;
} __attribute__ ((packed)) file_descr_t;

typedef struct {
    u32 prev_cluster;
} dir_handler_t;

typedef struct {
    ftype_t type;
    char name[MAX_FILE_NAME];
    u32 start_cluster;
    u32 curr_cluster;
    u32 curr_offset;
    u32 ent_cluster;
    u32 ent_offset;
    u32 ent_size;
    u8 mode;
    union {
        dir_handler_t;
        file_descr_t;
    };
} ft_entry_t;

typedef int fd_t;

ft_entry_t file_table[MAX_NB_FILE];

fd_t new_fd();
void free_fd(fd_t fd);
int check_fd(fd_t fd, ftype_t type);

typedef enum {
    SEEK_SET, SEEK_CUR, SEEK_END
} seek_cmd_t;

typedef struct {
    u32 cluster;
    u32 ent_offset;
    u32 ent_cluster;
    u32 ent_prev_cluster;
    char name[MAX_FILE_NAME];
    ftype_t type;
    u8 mode;
    u32 ent_size;   // Number of directory entries used by the file / directory.
    u32 size;       // Size of the file (0 for directories).
} dirent_t;

void init_fs(int show);
void load_mbr(int show);
void read_cluster(u32 cluster, u8* buffer);
void write_cluster(u32 cluster, u8 *buffer);
void get_short_name(directory_entry_t *dirent, char *buffer);
void get_long_name(long_file_name_t* lfn, char *buffer);
void set_long_name(long_file_name_t* lfn, char *buffer);
u32 get_next_cluster(u32 cluster);
void set_next_cluster(u32 cluster, u32 next_cluster);
u32 get_cluster(directory_entry_t *dirent);
u32 new_cluster();
void free_cluster(u32 cluster, u32 prev_cluster);
void free_cluster_chain(u32 start_cluster);
void reset_cluster(u32 cluster);
int new_entry(u32 cluster, dirent_t *dirent); // Fills the entry position fields
void free_entry(dirent_t *dirent); // frees corresponding entries
u32 insert_new_cluster(u32 prev, u32 next);

typedef struct {
    u32 size;   // Max size
    u32 index;  // Next free index
    u8 *buffer; // Address of the buffer
    fd_t file;  // File descriptor linked to stream
} stream_t;

#include "disk.h"
#include "partition.h"
#endif /* FILESYSTEM_H */

