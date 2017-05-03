#ifndef INT_H
#define INT_H
#define MAX_FILE_NAME 256
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

typedef u32 size_t;
typedef int ssize_t;

typedef int pid_t;
typedef int fd_t;

typedef enum {
    SEEK_SET, SEEK_CUR, SEEK_END
} seek_cmd_t;

typedef u8 oflags_t;

typedef enum {
    F_UNUSED, FILE, DIR
} ftype_t;

// TODO Find a smaller interesting struct for dirent_t !
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
#endif
