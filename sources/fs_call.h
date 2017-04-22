#ifndef FS_CALL_H
#define FS_CALL_H

#include "int.h"
#include "filesystem.h"
#include "file_name.h"
#include "lib.h"
#include "memory.h"

/*
 * Root directory doesn't have any "." or ".." entries.
 * Root directory cluster is written as "0" on the disk, so a cluster is 
 * considered as root cluster when cluster <= root_cluster
 */

fd_t openfile(char *path, oflags_t flags);
int close(fd_t fd);
int read(fd_t fd, void *buffer, int length);
int write(fd_t fd, void *buffer, int length);
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
dirent_t *findfile(fd_t dir, char *name);
dirent_t *findent(fd_t dir, char *name, ftype_t type);
int create_entries(fd_t dir, char *name, ftype_t type);
void set_size(fd_t fd, u32 size);
void test_dir();
void init_root();
#endif /* FS_CALL_H */

