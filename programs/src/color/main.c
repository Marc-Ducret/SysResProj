#include "lib.h"
#include "parsing.h"

char *available_colors_bg = "\
%bgBLACK%pbg           0       %bgDARK_GREY%pbg       8\n\
%bgBLUE%pbg            1       %bgLIGHT_BLUE%pbg      9\n\
%bgGREEN%pbg           2       %bgLIGHT_GREEN%pbg     10\n\
%bgCYAN%pbg            3       %bgLIGHT_CYAN%pbg      11\n\
%bgRED%pbg             4       %bgLIGHT_RED%pbg       12\n\
%bgMAGENTA%pbg         5       %bgLIGHT_MAGENTA%pbg   13\n\
%bgBROWN%pbg           6       %bgLIGHT_BROWN%pbg     14\n\
%bgLIGHT_GREY%pbg      7       %bgWHITE%pbg           15\n";

char *available_colors_fg = "\
%fgBLACK%pfg           0       %fgDARK_GREY%pfg       8\n\
%fgBLUE%pfg            1       %fgLIGHT_BLUE%pfg      9\n\
%fgGREEN%pfg           2       %fgLIGHT_GREEN%pfg     10\n\
%fgCYAN%pfg            3       %fgLIGHT_CYAN%pfg      11\n\
%fgRED%pfg             4       %fgLIGHT_RED%pfg       12\n\
%fgMAGENTA%pfg         5       %fgLIGHT_MAGENTA%pfg   13\n\
%fgBROWN%pfg           6       %fgLIGHT_BROWN%pfg     14\n\
%fgLIGHT_GREY%pfg      7       %fgWHITE%pfg           15\n";

void print_colors(int out, int bg) {
    fprintf(out, bg ? available_colors_bg : available_colors_fg, 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15); 
}

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
        print_colors(STDERR, bg);
        exit(EXIT_FAILURE);
    }
    
    errno = EINVAL;
    color = string_to_int(params[1]);
    if (color == -1 || color < 0 || color > 15) {
        fprintf(STDERR, "color: Invalid color argument: %s\n", params[1]);
        fprintf(STDERR, "The followings colors are supported:\n");
        print_colors(STDERR, bg);
        exit(EXIT_FAILURE);
    }
    
    if (bg)
        printf("%bg", color);
    else
        printf("%fg", color);
    exit(EXIT_SUCCESS);
    return 0;
}