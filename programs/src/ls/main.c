#include "lib.h"
#include "int.h"
#include "parsing.h"

char *sizes[4] = {"", "K", "M", "G"};

void print_size(size_t size) {
    int level = 0;
    if (size < 1000) {
        printf("%dB", size);
        return;
    }
    while (size >= 1000) {
        size = size / 10;
        level++;
    }
    int s = (level + 2) / 3;
    level = level % 4;
    printf("%d%s%d%s%d", size / 100, level==1?".":"", (size % 100) / 10, level==2?".":"", size % 10);
    printf("%sB", sizes[s]);
}

int main(char *a) {
    dirent_t dirent;
    args_t args;
    int res = parse(a, &args);
    if (res == -1)
        too_many_args("ls");

    int mode = eat_option(&args, "m");
    int size = eat_option(&args, "s");
    remain_option(&args, "ls");
    
    if (args.nb_args == 0) {
        args.nb_args = 1;
        args.args[0] = ".";
    }
    for (int i = 0; i < args.nb_args; i++) {
        char *file = args.args[i];
        fd_t fd = opendir(file);
        if (fd < 0) {
            fprintf(STDERR, "ls: Cannot open directory %s: %s\n", file, strerror(errno));
            continue;
        }
        int res = readdir(fd, &dirent);
        while (res == 0) {
            if (dirent.type == FILE) {
                printf("%s  ", dirent.name);
                if (mode || size) printf("(");
                if (size) print_size(dirent.size);
                if (mode && size) printf(", ");
                if (mode) {
                    printf("%s, %s", dirent.mode & SYSTEM ? "S":"U", 
                                      dirent.mode & RDONLY ? "RO":"RW");
                }
                if (mode || size) printf(")\n");
            }
            else {
                printf("%bg%s%pbg  ", BLUE, dirent.name);
                if (mode)
                    printf("(%s, %s)", dirent.mode & SYSTEM ? "S":"U", 
                                      dirent.mode & RDONLY ? "RO":"RW");
                if (mode || size)
                    printf("\n");
            }
            res = readdir(fd, &dirent);
        }
        int err = errno;
        closedir(fd);
        if (err != ENOENT) {
            fprintf(STDERR, "\nls: An error occured while reading %s: %s\n", file, strerror(errno));
            continue;
        }
        if (!mode && !size) printf("\n");
        if (i < args.nb_args - 1)
            printf("\n");
    }
    flush(STDOUT);
    exit(EXIT_SUCCESS);
    return 0;
}
