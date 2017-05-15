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

char *concat(char *sa, char *sb) {
    u32 la = strlen(sa);
    u32 lb = strlen(sb);
    char *s = (char *) malloc(la + lb + 2);
    memcpy(s, sa, la);
    s[la] = '/';
    memcpy(s + la + 1, sb, lb);
    s[la + lb + 1] = 0;
    return s;
}

void print_folder(char *file, int mode, int size, int offset) {
    dirent_t dirent;
    fd_t fd = opendir(file);
    if (fd < 0) {
        fprintf(STDERR, "tree: Cannot open directory %s: %s\n", file, strerror(errno));
        return;
    }
    int res = readdir(fd, &dirent);
    int err;

    while (res == 0) {
        dirent_t next;
        res = readdir(fd, &next);
        err = errno;
        
        if(dirent.name[0] != '.') {
            if (dirent.type == FILE) {
                for(int i = 0; i < offset; i ++) printf("\xB3 ");
                printf("%c%s", res ? 0xC0 : 0xC3, dirent.name);
                if (mode || size) printf(" (");
                if (size) print_size(dirent.size);
                if (mode && size) printf(", ");
                if (mode) {
                    printf("%s, %s", dirent.mode & SYSTEM ? "S":"U", 
                                      dirent.mode & RDONLY ? "RO":"RW");
                }
                if (mode || size) printf(")");
                printf("\n");
            } else {
                for(int i = 0; i < offset; i ++) printf("\xB3 ");
                printf("%c\xC4\xC2%fg%s%pfg", res ? 0xC0 : 0xC3, BLUE, dirent.name);
                if (mode)
                    printf(" (%s, %s)", dirent.mode & SYSTEM ? "S":"U", 
                                      dirent.mode & RDONLY ? "RO":"RW");
                printf("\n");
                if(dirent.name[0] != '.') {
                    char *subdir = concat(file, dirent.name);
                    print_folder(subdir, mode, size, offset+1);
                    free(subdir);
                }
            }
        }
        dirent = next;
    }
    closedir(fd);
    if (err != ENOENT) {
        fprintf(STDERR, "\ntree: An error occured while reading %s: %s\n", file, strerror(errno));
        return;
    }
}

int main(char *a) {
    args_t args;
    int res = parse(a, &args);
    if (res == -1)
        too_many_args("tree");

    int mode = eat_option(&args, "m");
    int size = eat_option(&args, "s");
    remain_option(&args, "t");
    
    print_folder(".", mode, size, 0);
    
    flush(STDOUT);
    exit(EXIT_SUCCESS);
    return 0;
}
