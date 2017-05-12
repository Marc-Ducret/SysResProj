#include "lib.h"

int main(char *args) {
    dirent_t dirent;
    if (!*args) {
        strCopy(".", args);
    }
    fd_t fd = opendir(args);
    if (fd < 0) {
        fprintf(STDERR, "ls: Cannot open directory / %s: %s\n", args, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int res = readdir(fd, &dirent);
    while (res == 0) {
        printf("%s  ", dirent.name);
        res = readdir(fd, &dirent);
    }
    int err = errno;
    closedir(fd);
    if (err != ENOENT) {
        fprintf(STDERR, "\nls: An error occured while reading %s: %s\n", args, strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("\n");
    flush(STDOUT);
    exit(EXIT_SUCCESS);
    return 0;
}
