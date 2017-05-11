#include "lib.h"
#include "parsing.h"

char *available_colors = "\
BLACK           0       DARK_GREY       8\n\
BLUE            1       LIGHT_BLUE      9\n\
GREEN           2       LIGHT_GREEN     10\n\
CYAN            3       LIGHT_CYAN      11\n\
RED             4       LIGHT_RED       12\n\
MAGENTA         5       LIGHT_MAGENTA   13\n\
BROWN           6       LIGHT_BROWN     14\n\
LIGHT_GREY      7       WHITE           15\n";

int main(char *args) {
    char *params[2];
    int color;
    int bg = 1;
    int nb_args = get_args(args, params, 2);

    if (nb_args == -1) {
        too_many_args("color");
        return 0;
    }
    if (nb_args == 0) {
        fprintf(STDERR, "color: Missing arguments\n");
        exit(EXIT_FAILURE);
    }
    
    if (strEqual(params[0], "fg"))
        bg = 0;
    else if (!strEqual(params[0], "bg")) {
        fprintf(STDERR, "color: Invalid argument: %s\n", params[0]);
        fprintf(STDERR, "Expected fg (foreground) or bg (background)\n");
        exit(EXIT_FAILURE);
    }
    
    if (nb_args == 1) {
        fprintf(STDERR, "color: Missing color argument: the followings colors are supported:\n");
        fprintf(STDERR, "%s", available_colors);
        exit(EXIT_FAILURE);
    }
    
    errno = EINVAL;
    color = string_to_int(params[1]);
    if (color == -1 || color < 0 || color > 15) {
        fprintf(STDERR, "color: Invalid color argument: %s\n", params[1]);
        fprintf(STDERR, "The followings colors are supported:\n");
        fprintf(STDERR, "%s", available_colors);
        exit(EXIT_FAILURE);
    }
    
    if (bg)
        printf("%bg", color);
    else
        printf("%fg", color);
    exit(EXIT_SUCCESS);
    return 0;
}