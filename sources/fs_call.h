#ifndef FS_CALL_H
#define FS_CALL_H

#include "int.h"
#include "filesystem.h"
#include "file_name.h"
#include "lib.h"
#include "memory.h"

typedef struct {
    u32 cluster;
    char name[MAX_FILE_NAME];
    ftype_t type;
} dirent_t;

fd_t openfile(char *path, oflags_t flags);
int close(fd_t fd);
int read(fd_t fd, void *buffer, int offset, int length);
int write(fd_t fd, void *buffer, int offset, int length);
int seek(fd_t fd, seek_cmd_t seek_command, int offset);

void mkdir(char *path);
void rmdir(char *path);
void chdir(char *path);
char *getcwd();
fd_t opendir(char *path);
fd_t opendir_ent(dirent_t *dirent);
dirent_t *readdir(fd_t fd);
void rewinddir(fd_t fd);
void closedir(fd_t fd);
dirent_t *finddir(fd_t dir, char *name);
dirent_t *cluster_finddir(fd_t dir, u32 cluster);
void test_dir();
#endif /* FS_CALL_H */

