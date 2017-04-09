#include "file_name.h"

// TODO malloc, details witt trailing '/' and root '/' ?
// Below these are very naive implementations !

char *basename(char *path) {
    char *p = path;
    for (; *p; p++) {}
    for (; (p >= path) && (*p != DIR_SEP); p--) {}
    
    if (p < path) {
        // No DIR_SEP.
        return path;
    }
    else {
        *p = 0;
        return p + 1;
    }
}

char *dirname(char * path) {
    char *p = path;
    for (; *p; p++) {}
    for (; (p >= path) && (*p != DIR_SEP); p--) {}
    
    if (p < path) {
        // No DIR_SEP.
        return "."; // TODO Malloc ?
    }
    else {
        *p = 0;
        return path;
    }
}

char *nextdirname(char *path, char** res_pointer) {
    // Splits the path into the first directory name, and the remaining path.
    // Returns a pointer to remaining path
    // Uses res_pointer to put the pointer towards the first directory name.
    // res_pointer is always equal to path. TODO
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