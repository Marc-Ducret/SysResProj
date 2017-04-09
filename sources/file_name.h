#ifndef FILE_NAME_H
#define FILE_NAME_H
#include "lib.h"

#define DIR_SEP '/'
#define CUR_DIR_NAME "."
#define PARENT_DIR_NAME ".."

char *basename(char *path);
char *dirname(char *path);
char *nextdirname(char *path, char** res_pointer);
void append(char *first, char *second) {
    // Concatenates the two names in the first buffer, with directory separator.
    while (*first != 0)
        first++;
    *first = DIR_SEP;
    while (*second != 0)
        *first++ = *second++;
    
    *first = 0;
}
#endif /* FILE_NAME_H */

