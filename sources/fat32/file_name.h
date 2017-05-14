#ifndef FILE_NAME_H
#define FILE_NAME_H
#include "lib.h"

#define DIR_SEP '/'
#define CUR_DIR_NAME "."
#define PARENT_DIR_NAME ".."
#define DIR_SEP_STR "/"
#define ROOT_NAME '/'
#define ROOT_NAME_STR "/"

#include "filesystem.h"

char *basename(char *path);
char *dirname(char *path);
char *nextdirname(char *path, char** res_pointer);
void concat(char *dir, char *file);

#endif /* FILE_NAME_H */

