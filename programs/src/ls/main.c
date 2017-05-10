#include "lib.h"

int main(char *args) {
    dirent_t dirent;
    if (!*args) {
        strCopy(".", args);
    }
    fd_t fd = opendir(args);
    if (fd < 0) {
        fprintf(STDERR, "ls: Cannot open directory / %s: %s\n", args, strerror(errno));
        return 1;
    }
    int res = readdir(fd, &dirent);
    while (res == 0) {
        printf("%s  ", dirent.name);
        res = readdir(fd, &dirent);
    }
    if (errno != ENOENT) {
        fprintf(STDERR, "\nls: An error occured while reading %s: %s\n", args, strerror(errno));
    }
    closedir(fd);
    flush(STDOUT);
    return 0;
}