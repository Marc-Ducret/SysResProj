#ifndef PARSING_H
#define PARSING_H
#include "lib.h"

#define NUM_ARGS_MAX 50
#define NUM_OPTS_MAX 50

typedef struct {
    int nb_args;
    int nb_opts;
    char *args[NUM_ARGS_MAX];
    char *opts[NUM_OPTS_MAX];
} args_t;

char *parse_arg(char *src, char **first);
char *split_args(char *src, char **table, size_t size);
int get_args(char *src, char **table, size_t size);
int parse(char *data, args_t *args);
int string_to_int(char *src);
void too_many_args(char *fun);
int remain_option(args_t *args, char *fun);
int eat_option(args_t *args, char *option);
#endif /* PARSING_H */

