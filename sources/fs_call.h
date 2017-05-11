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
fd_t cwd;

fd_t fopen(char *path, oflags_t flags);
int close(fd_t fd);
ssize_t read(fd_t fd, void *buffer, size_t length);
ssize_t write(fd_t fd, void *buffer, size_t length);
int seek(fd_t fd, seek_cmd_t seek_command, int offset); // TODO off_t ?
int remove(char *path);
int copyfile(char *old_path, char *new_path);

int mkdir(char *path, u8 mode);
int rmdir(char *path);
int chdir(char *path);
char *getcwd();
fd_t opendir(char *path);
fd_t opendir_ent(dirent_t *dirent);
dirent_t *readdir(fd_t fd);
int rewinddir(fd_t fd);
int closedir(fd_t fd);
dirent_t *finddir(fd_t dir, char *name);
dirent_t *cluster_finddir(fd_t dir, u32 cluster);
dirent_t *findfile(fd_t dir, char *name);
dirent_t *findent(fd_t dir, char *name, ftype_t type);
int create_entries(fd_t dir, char *name, ftype_t type, u8 mode);
void set_size(fd_t fd, size_t size);
void test_dir();
int init_root();
int init_filename_gen();
int save_filename_gen();
#endif /* FS_CALL_H */

