#ifndef PARSING_H
#define PARSING_H
#include "lib.h"

char *parse_arg(char *src, char **first);
char *split_args(char *src, char **table, size_t size);
int get_args(char *src, char **table, size_t size);
int string_to_int(char *src);
void too_many_args(char *fun);

#endif /* PARSING_H */

