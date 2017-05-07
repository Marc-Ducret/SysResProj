#ifndef INT_H
#define INT_H

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

#define MAX_FILE_NAME 256
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

typedef struct {
    int mseconds;
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
    int century;
} rtc_time_t;

#endif
