#include "file_name.h"

// TODO malloc, details witt trailing '/' and root '/' ?
// Below these are very naive implementations !
char static_dirname[MAX_PATH_NAME];
char static_filename[MAX_PATH_NAME];

char *basename(char *path) {
    //TODO with malloc
    // The three functions coming use a single static buffer !
    char *p = path;
    for (; *p; p++) {}
    p--;
    for (; *p == DIR_SEP; p--);
    *(p + 1) = 0;
    for (; (p >= path) && (*p != DIR_SEP); p--);
    if (p < path) {
        // No DIR_SEP.
        strCopy(path, static_filename);
        return static_filename;
    }
    else {
        strCopy(p + 1, static_filename);
        return static_filename;
    }
}

char *dirname(char *path) {
    strCopy(path, static_dirname);
    
    char *p = static_dirname;
    for (; *p; p++);
    p--;
    for (; *p == DIR_SEP; p--);
    *(p + 1) = 0;
    for (; (p >= static_dirname) && (*p != DIR_SEP); p--) {}
    
    if (p < static_dirname) {
        // No DIR_SEP.
        strCopy(CUR_DIR_NAME, static_dirname);
    }
    else if (p > static_dirname) { 
        *p = 0; // Keeps the part at the right of this separator.
    }
    else {
        *(p+1) = 0; // This separator means root directory !
    }
    return static_dirname;
}

char *nextdirname(char *path, char** res_pointer) {
    // Splits the path into the first directory name, and the remaining path.
    // Returns a pointer to remaining path.
    // Uses res_pointer to put the pointer towards the first directory name.
    // WARNING : this uses the arg string to store result !
    char *p = path;
    for (; *p && (*p != DIR_SEP); p++) {}
    if (*p) {
        *res_pointer = path;
        *p = 0;
        return p + 1;
    }
    else {
        // No directory separator.
        *res_pointer = path;
        return p;
    }
}

void concat(char *dir, char *file) {
    // Concatenates the two names in the dir buffer, with directory separator.
    while (*dir != 0)
        dir++;
    *dir = DIR_SEP;
    dir++;
    
    while (*file != 0)
        *dir++ = *file++;
    
    *dir = 0;
}