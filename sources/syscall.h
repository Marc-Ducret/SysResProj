#ifndef SYSCALL_H
#define SYSCALL_H

// Process related System Calls
#define KNEWCHANNEL 0
#define KSEND 1
#define KRECEIVE 2
#define KFORK 3
#define KEXIT 4
#define KWAIT 5

pid_t kfork(priority prio);
pid_t kwait(int *status);
void kexit(int status);

// Utilisera-t-on des file descriptors ? TODO
int ksend(chanid channel, int msg);
int kreceive(chanid channel[4], int *dest);
chanid knew_channel();

// File System related Calls
#define KFOPEN 10
#define KCLOSE 11
#define KREAD 12
#define KWRITE 13
#define KSEEK 14

#define KMKDIR 20
#define KRMDIR 21
#define KCHDIR 22
#define KGETCWD 23
#define KOPENDIR 24
#define KREADDIR 25
#define KREWINDDIR 26
#define KCLOSEDIR 27

#define KGET_KEY_EVENT 40

fd_t kfopen(char *path, oflags_t flags);
int kclose(fd_t fd);
ssize_t kread(fd_t fd, void *buffer, size_t length);
ssize_t kwrite(fd_t fd, void *buffer, size_t length);
int kseek(fd_t fd, seek_cmd_t seek_command, int offset);

int kmkdir(char *path, u8 mode);
int krmdir(char *path);
int kchdir(char *path);
char *kgetcwd();
fd_t kopendir(char *path);
dirent_t *kreaddir(fd_t fd);
int krewinddir(fd_t fd);
int kclosedir(fd_t fd);

int k_get_key_event();

void new_launch();
#endif /* SYSCALL_H */

