#include "lib.h"
#include "parsing.h"

#define CMD_SIZE 0x200
#define NB_SHELL_CMD 27
char *shell_commands[NB_SHELL_CMD] =  {
    "ls",
    "mkdir",
    "rmdir",
    "cp",
    "rm",
    "mv",
    "ps",
    "sl",
    "pwd",
    "cat",
    "touch",
    "kill",
    "color",
    "clear",
    "cacatoes"
};

#define SHELL_CMD_PATH "/"
#define SHELL_CMD_EXT ".bin"

u8 recv_buff[512];

char cmd[CMD_SIZE];
char cwd[MAX_PATH_NAME];
int pos;
int run;

void new_cmd() {
    printf("%fgwhateveryouwant@CacatOez:%pfg%fg%s%pfg>", GREEN, BLUE, cwd);
    flush(STDOUT);
}

int exec_builtin(char *fun, char *args) {
    
    if(strEqual(fun, "help")) {
        printf("Available commands: help, exit, int, test\n");
    }
    else if(strEqual(fun, "exit")) {
        char *exit_string;
        int exit_value = 0;
        int nb_args = get_args(args, &exit_string, 1);
        
        if (nb_args == -1) {
            too_many_args(fun);
            return 0;
        }
        if (nb_args == 1) {
            exit_value = string_to_int(exit_string);
        }
        exit(exit_value);
    }
    else if (strEqual(fun, "pwd")) {
        if (*args) {
            fprintf(STDERR, "pwd: Too many arguments");
        }
        else {
            int res = getcwd(cwd);
            if (res == -1) {
                fprintf(STDERR, "pwd: Couldn't print current working directory: %s", 
                        strerror(errno));
            }
            else {
                printf("%s", cwd);
            }
        }
    }
    else if (strEqual(fun, "cd")) {
        char *path;
        int nb_args = get_args(args, &path, 1);
        if (nb_args == -1) {
            fprintf(STDERR, "cd: Too many arguments");
        }
        else {
            if (nb_args == 0)
                path = ROOT_NAME_STR;
            int res = chdir(path);
            if (res == -1)
                fprintf(STDERR, "cd: Couldn't change directory to %s: %s", path,
                        strerror(errno));
            else {
                getcwd(cwd);
            }
        }
    }
    else if (strEqual(fun, "clear")) {
        esc_seq(ESC_CLEAR);
    }
    else {
        // Not a known builtin.
        return -1;
    }
    return 0;
}

int exec_file(char *file, char *args) {
    printf("Exec file <%s>\n", file);
    if(exec(file, args, STDIN, STDOUT) < 0) // TODO Args ?
        printf("Error: %s\n", strerror(errno));
    else {
        int status;
        wait(&status);
    }
    return 0;
}

int exec_shell_cmd(char *fun, char *args) {
    int i;
    for (i = 0; i < NB_SHELL_CMD; i++) {
        if (shell_commands[i] && strEqual(shell_commands[i], fun))
            break;
    }
    if (i == NB_SHELL_CMD)
        return -1;
    
    char file[MAX_FILE_NAME];
    strCopy(SHELL_CMD_PATH, file);
    int offset = strlen(SHELL_CMD_PATH);
    strCopy(fun, &file[offset]);
    offset += strlen(fun);
    strCopy(SHELL_CMD_EXT, &file[offset]);
    exec_file(file, args);
    return 0;
}

void exec_cmd() {
    errno = ECLEAN;
    char *first;
    char *args = parse_arg(cmd, &first);
    args = args ? args : "";
    
    if (first != NULL) {
        if (exec_builtin(first, args) != 0) {
            if(strEqual(first, "test")) {
                for(int i = 0; i < 100; i ++)
                    printf("%d\n", i);
            } else if(strEqual(first, "int")) {
                asm volatile ("int $13");
            } else if(first[0] == '/' || first[0] == '.') {
                // Execute a file.
                exec_file(first, args);
            }
            else {
                if (exec_shell_cmd(first, args) == -1)
                    printf("Unknown command (%s)", cmd);
            }
        }
    }
    while(pos > 0)
        cmd[--pos] = 0;
    new_cmd();
}

void key_typed(u8 c) {
    if(c == '\n' || pos == CMD_SIZE-1) {
        stream_putchar(c, STDOUT);
        flush(STDOUT);
        exec_cmd();
    } else if(c == 0x8) {
        if(pos > 0) {
            cmd[--pos] = 0;
            stream_putchar(c, STDOUT);
        }
    } else {
        cmd[pos++] = c;
        stream_putchar(c, STDOUT);
    }
}

int main() {
    run = 1;
    memset(cwd, 0, MAX_PATH_NAME);
    getcwd(cwd);
    new_cmd();
    while(run) {
        int ct;
        sleep(50);
        if((ct = receive(0, recv_buff, 512)) > 0) {
            for(int i = 0; i < ct; i++)
                key_typed(recv_buff[i]);
            flush(STDOUT);
        }
    }
}
