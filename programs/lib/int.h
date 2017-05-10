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

#define DIR_SEP '/'
#define CUR_DIR_NAME "."
#define PARENT_DIR_NAME ".."
#define DIR_SEP_STR "/"
#define ROOT_NAME '/'
#define ROOT_NAME_STR "/"

typedef enum {
    F_UNUSED, FILE, DIR
} ftype_t;

// TODO Find a smaller interesting struct for dirent_t !
typedef struct {
    char name[MAX_FILE_NAME];
    ftype_t type;
    u8 mode;
    u32 size;
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

typedef enum p_state 
{
    P_FREE, P_BLOCKEDWRITING, P_BLOCKEDREADING, P_WAITING, P_RUNNABLE, P_ZOMBIE, P_SLEEPING
} p_state_t;

typedef struct {
    pid_t pid;
    pid_t parent;
    char name[256];
    p_state_t p_state;
} process_info_t;

#endif
