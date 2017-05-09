#ifndef SYSCALL_H
#define SYSCALL_H
#include "int.h"
#include "error.h"
#include "stream.h"
// Process related System Calls
#define NEWCHANNEL 0
#define SEND 1
#define RECEIVE 2
#define EXEC 3
#define EXIT 4
#define WAIT 5
#define FREECHANNEL 6
#define WAITCHANNEL 7
#define SLEEP 8

pid_t exec(char *cmd, int chin, int chout);
pid_t wait(int *status);
void exit(int status);

ssize_t send(int chanid, u8 *buffer, size_t len);
ssize_t receive(int chanid, u8 *buffer, size_t len);
ssize_t wait_channel(int chanid, int write);
int new_channel(void);
int free_channel(int chanid);

int sleep(int time);

// File System related Calls
#define FOPEN 10
#define CLOSE 11
#define READ 12
#define WRITE 13
#define SEEK 14

#define MKDIR 20
#define RMDIR 21
#define CHDIR 22
#define GETCWD 23
#define OPENDIR 24
#define READDIR 25
#define REWINDDIR 26
#define CLOSEDIR 27

fd_t fopen(char *path, oflags_t flags);
int close(fd_t fd);
ssize_t read(fd_t fd, void *buffer, size_t length);
ssize_t write(fd_t fd, void *buffer, size_t length);
int seek(fd_t fd, seek_cmd_t seek_command, int offset);

int mkdir(char *path, u8 mode);
int rmdir(char *path);
int chdir(char *path);
char *kgetcwd();
fd_t opendir(char *path);
dirent_t *kreaddir(fd_t fd);
int rewinddir(fd_t fd);
int closedir(fd_t fd);

// Other system calls

#define GET_KEY_EVENT 40
#define GET_TIME_OF_DAY 41
int get_key_event();
int gettimeofday(rtc_time_t *time);
void *resize_heap(int delta_size);

#endif /* SYSCALL_H */

