#include "parsing.h"

char *parse_arg(char *src, char **first) {
    // Parses the first argument of the input string, and returns a pointer to
    // the remaining args.
    // If there isn't any arg, returns NULL, instead of a NULL string.
    while (*src == ' ')
        src++;
    if (*src)
        *first = src;
    else {
        first = NULL;
        return NULL;
    }
    
    while (*src && *src != ' ')
        src++;
    
    if (!*src) {
        return NULL;
    }
    *src = 0;
    src++;
    while (*src == ' ')
        src++;
    if (*src)
        return src;
    return NULL;
}

char *split_args(char *src, char**table, size_t size) {
    // Parses at most size args of input src into array table, and returns the
    // remaining string.
    // Returns NULL instead of NULL string if no arg left.
    while (*src == ' ')
        src++;
    
    for (int i = 0; i < size; i++) {
        table[i] = NULL;
    }
    while (size > 0 && src != NULL) {
        src = parse_arg(src, table++);
        size--;
    }
    if (src && (!*src))
        src = NULL;
    return src;
}

int get_args(char *src, char **table, size_t size) {
    // Tries to parses at most size arguments.
    // Returns the number of parsed args, or -1 if there was too many.
    src = split_args(src, table, size);
    if (src)
        return -1;
    int res = 0;
    while (res < size && table[res] != NULL)
        res++;
    return res;
}

int string_to_int(char *src) {
    // Tries to parse an int.
    // Suprisingly returns -1 if not an integer.
    int res = 0;
    int minus = 0;
    if (*src == '-') {
        minus = 1;
        src++;
    }

    char c = *src;
    while (c && ('0' <= c) && (c <= '9')) {
        res = 10 * res + (c - '0');
        c = *(++src);
    }
    if (c)
        return -1;
    return minus ? (-res) : res;
}

void too_many_args(char *fun) {
    fprintf(STDERR, "%s: Too many arguments", fun);
    exit(EXIT_FAILURE);
}