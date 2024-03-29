#include "lib.h"
#include "parsing.h"

#define CMD_SIZE 0x200
#define HISTORY_LENGTH 512

#define NB_SHELL_CMD 23
#define NB_BUILTIN 7

char *shell_commands[NB_SHELL_CMD] =  {
    "ls",
    "tree",
    "mkdir",
    "rmdir",
    "cp",
    "rm",
    "mv",
    "ps",
    "sl",
    "cat",
    "touch",
    "kill",
    "color",
    "clear",
    "cacatoes",
    "fwrite",
    "time",
    "shell",
    "console",
    "sl",
    "splash",
    "more",
    "tron"
};

char *builtin_commands[NB_BUILTIN] = {
    "cd",
    "set-user",
    "exit",
    "help",
    "pwd",
    "clear",
    "clock"
};

#define SHELL_CMD_PATH "/bin/"
#define SHELL_CMD_EXT ".bin"
#define SHELL_PATH "/bin/shell.bin"

u8 recv_buff[512];

char cmd[CMD_SIZE];
char cwd[MAX_PATH_NAME];
char *history[HISTORY_LENGTH];
int last_cmd;
int cur_cmd;
int first_cmd;
char user_name[512];
char *default_user = "ez";
int pos = 0;
int run;
int ephemeral = 0;

void new_cmd() {
    printf("%fg%s%pfg%fg@%pfg%fgCacatOS:%pfg%fg%s%pfg>",
            GREEN, user_name, BLUE, GREEN, BLUE, cwd);
    flush(STDOUT);
}

int exec_builtin(char *fun, char *args) {
    
    if(strEqual(fun, "help")) {
        printf("Available BuiltIns:\n");
        for (int i = 0; i < NB_BUILTIN; i++) {
            if (builtin_commands[i])
                printf("%fg%s\n%pfg", GREEN, builtin_commands[i]);
        }
        printf("\nAvailable commands:\n");
        for (int i = 0; i < NB_SHELL_CMD; i++) {
            if (shell_commands[i])
                printf("%fg%s\n%pfg", BLUE, shell_commands[i]);
        }
        printf("\nUsage : %fgcmd     args     < in     >/>> out     (&)%pfg\n", BLUE);
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
            fprintf(STDERR, "pwd: Too many arguments\n");
        }
        else {
            int res = getcwd(cwd);
            if (res == -1) {
                fprintf(STDERR, "pwd: Couldn't print current working directory: %s\n", 
                        strerror(errno));
            }
            else {
                printf("%s\n", cwd);
            }
        }
    }
    else if (strEqual(fun, "cd")) {
        char *path;
        int nb_args = get_args(args, &path, 1);
        if (nb_args == -1) {
            fprintf(STDERR, "cd: Too many arguments\n");
        }
        else {
            if (nb_args == 0)
                path = ROOT_NAME_STR;
            int res = chdir(path);
            if (res == -1)
                fprintf(STDERR, "cd: Couldn't change directory to %s: %s\n", path,
                        strerror(errno));
            else {
                getcwd(cwd);
            }
        }
    }
    else if (strEqual(fun, "clear")) {
        esc_seq(ESC_CLEAR);
    }
    else if (strEqual(fun, "set-user")) {
        if (!*args)
            args = default_user;
        strCopy(args, user_name);
    }
    else if (strEqual(fun, "clock")) {
        esc_seq(ESC_CLOCK);
    }
    else {
        // Not a known builtin.
        return -1;
    }
    return 0;
}

int exec_file(char *file, char *args, int bg) {
    //printf("Exec file <%s> with args <%s> and bg <%d>\n", file, args, bg);
    if(exec(file, args, STDIN, STDOUT) < 0)
        printf("Error: %s\n", strerror(errno));
    else if (!bg) {
        int status;
        if (ephemeral) {
            free_channel(STDIN);
            free_channel(STDOUT);
        }
        wait(&status);
    }
    return 0;
}

int exec_shell_cmd(char *fun, char *args, int bg) {
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
    exec_file(file, args, bg);
    return 0;
}

int exec_simple_cmd(char *s, int bg) {
    errno = ECLEAN;
    char *first;
    char *args = parse_arg(s, &first);
    args = args ? args : "";
    //printf("Executing simple cmd <%s> with args <%s>\n", first, args);
    //flush(STDOUT);
    if (first != NULL) {
        if (exec_builtin(first, args) != 0) {
            if(strEqual(first, "test")) {
                for(int i = 0; i < 100; i ++)
                    printf("%d\n", i);
            } else if(strEqual(first, "int")) {
                asm volatile ("int $13");
            } else if(first[0] == '/' || first[0] == '.') {
                // Execute a file.
                return exec_file(first, args, bg);
            }
            else if (strEqual(first, "t")) {
                _test();
            }
            else {
                if (exec_shell_cmd(first, args, bg) == -1) {
                    fprintf(STDERR, "Unknown command (%s)\n", first);
                    return -1;
                }
            }
        }
    }
    return 0;
}

char *strip(char *s) {
    // Removes ending spaces and returns pointer to the end.
    char *cur = s;
    while (*cur)
        cur++;
    cur--;
    while (cur >= s && *cur == ' ')
        *cur-- = 0;
    if (cur >= s)
        return cur;
    return s;
}

int exec_pipe(char *cmd1, char *cmd2, int no_wait) {
    //printf("Executing some pipe\n");
    //flush(STDOUT);
    strip(cmd1);
    strip(cmd2);
    if (!*cmd1 || !*cmd2) {
        fprintf(STDERR, "pipe: Syntax error\n");
        return -1;
    }
    int chan = new_channel();
    if (chan == -1) {
        fprintf(STDERR, "pipe: Unable to create a channel: %s\n", strerror(errno));
        return -1;
    }
    int res1 = exec(SHELL_PATH, cmd1, STDIN, chan);
    if (res1 == -1) {
        fprintf(STDERR, "pipe: Unable to exec: %s\n", strerror(errno));
    }
    //sleep(50);
    int res2 = exec(SHELL_PATH, cmd2, chan, STDOUT);
    if (res2 == -1) {
        fprintf(STDERR, "pipe: Unable to exec: %s\n", strerror(errno));
    }
    free_channel(chan);
    if (ephemeral) {
        free_channel(STDIN);
        free_channel(STDOUT);
    }
    if (!no_wait && res1 >= 0) {
        wait(NULL);
    }
    if (!no_wait && res2 >= 0) {
        wait(NULL);
    }
    return 0;
}

int exec_redir(char *s, int bg) {
    //printf("searching some redirection\n");
    //flush(STDOUT);
    char *cur = s;
    while (*cur) {        
        if (*cur == '>') {
            int append = 0;
            *cur = 0;
            cur++;
            if (*cur == '>') {
                *cur = 0;
                cur++;
                append = 1;
            }
            //printf("Found a redirection to %s\n", cur);
            // Executes '| fwrite' instead
            char *cmd2 = malloc(MAX_PATH_NAME);
            if (cmd2 == NULL) {
                fprintf(STDERR, "redirection >: Out of memory\n");
                return -1;
            }
            strCopy(SHELL_CMD_PATH, cmd2);
            int offset = strlen(SHELL_CMD_PATH);
            strCopy("fwrite", &cmd2[offset]);
            offset += strlen("fwrite");
            strCopy(SHELL_CMD_EXT, &cmd2[offset]);
            offset += strlen(SHELL_CMD_EXT);
            cmd2[offset] = ' ';
            offset++;
            if (append) {
                strCopy("-a ", &cmd2[offset]);
                offset += strlen("-a ");
            }
            strCopy(cur, &cmd2[offset]);
            int res = exec_pipe(s, cmd2, bg);
            free(cmd2);
            return res;
        }
        if (*cur == '<') {
            *cur = 0;
            cur++;
            //printf("Found a redirection to %s\n", cur);
            // Executes 'cat * |' instead
            char *cmd1 = malloc(MAX_PATH_NAME);
            if (cmd1 == NULL) {
                fprintf(STDERR, "redirection <: Out of memory\n");
                return -1;
            }
            strCopy(SHELL_CMD_PATH, cmd1);
            int offset = strlen(SHELL_CMD_PATH);
            strCopy("cat", &cmd1[offset]);
            offset += strlen("cat");
            strCopy(SHELL_CMD_EXT, &cmd1[offset]);
            offset += strlen(SHELL_CMD_EXT);
            cmd1[offset] = ' ';
            offset++;
            strCopy(cur, &cmd1[offset]);
            int res = exec_pipe(cmd1, s, bg);
            free(cmd1);
            return res;
        }
        cur++;  
    }
    // No redirections
    return exec_simple_cmd(s, bg);
}

int exec_cmd(char *s) {
    // Search for seq ';'
    char *cur = s;
    while(*cur) {
        if (*cur == ';') {
            *cur = 0;
            cur++;
            exec_cmd(s);
            return exec_cmd(cur);
        }
        cur++;
    }
    // Search for pipes
    cur = s;
    while(*cur) {
        if (*cur == '|') {
            *cur = 0;
            cur++;
            return exec_pipe(s, cur, 0);
        }
        cur++;
    }
    //printf("No pipes, searching for &\n");
    //flush(STDOUT);
    // It is a simple command.
    // Checks for an ampersand
    char *last_ch = strip(s);
    
    if (*last_ch == '&') {
        if (ephemeral) {
            // We have to execute this command and die.
            *last_ch = 0;
            //printf("We have to exec this cmd and die\n");
            //flush(STDOUT);
            return exec_redir(s, 1);
        }
        else {
            //printf("Found a &: forking an ephemeral shell\n");
            //flush(STDOUT);
            int res = exec(SHELL_PATH, s, -1, STDOUT);
            if (res == -1) {
                fprintf(STDERR, "bg-task: Unable to exec: %s\n", strerror(errno));
                return -1;
            }
            wait(NULL);
            return 0;
        }
    }
    //printf("No &\n");
    return exec_redir(s, 0);
}

void save_cmd() {
    int prev_cmd = (last_cmd + HISTORY_LENGTH - 1) % HISTORY_LENGTH;
    if (last_cmd == first_cmd || !strEqual(history[prev_cmd], cmd)) {
        size_t len = strlen(cmd);
        history[last_cmd] = malloc(len + 1);
        if (history[last_cmd]) {
            strCopy(cmd, history[last_cmd]);
            last_cmd = (last_cmd + 1) % HISTORY_LENGTH;
        }
        if (history[last_cmd]) {
            free(history[last_cmd]);
            history[last_cmd] = NULL;
            first_cmd = (first_cmd + 1) % HISTORY_LENGTH;
        }
    }
    cur_cmd = last_cmd;
}

void clear_cmd() {
    while(pos > 0)
        cmd[--pos] = 0;
    new_cmd();
}

void complete(char *path);
void complete_cmd(char *start);

void key_typed(u8 c) {
    if(c == '\n' || pos == CMD_SIZE-1) {
        stream_putchar(c, STDOUT);
        flush(STDOUT);
        char *end = strip(cmd);
        if (*end) {
            save_cmd();
            exec_cmd(cmd);
        }
        clear_cmd();
    } else if(c == 0x8) {
        if(pos > 0) {
            cmd[--pos] = 0;
            stream_putchar(c, STDOUT);
        }
    } else if (c == CHAR_UP) {
        if (cur_cmd == first_cmd)
            return;
        
        cur_cmd = (cur_cmd + HISTORY_LENGTH - 1) % HISTORY_LENGTH;
        while (pos > 0) {
            cmd[--pos] = 0;
            stream_putchar(0x8, STDOUT);
        }
        if (history[cur_cmd]) {
            char *s = history[cur_cmd];
            while (*s) {
                c = *s++;
                cmd[pos++] = c;
                stream_putchar(c, STDOUT);
            }
        }
    } else if (c == CHAR_DOWN) {
        if (cur_cmd == last_cmd) {
            return;
        }
        while (pos > 0) {
            cmd[--pos] = 0;
            stream_putchar(0x8, STDOUT);
        }
        
        cur_cmd = (cur_cmd + 1) % HISTORY_LENGTH;
        if (history[cur_cmd]) {
            char *s = history[cur_cmd];
            while (*s) {
                c = *s++;
                cmd[pos++] = c;
                stream_putchar(c, STDOUT);
            }
        }
    }else if (c == CHAR_LEFT || c == CHAR_RIGHT) {
        // Nothing to do yet.
    }
    else if (c == '\t') {
        if (pos == 0)
            return;
        
        int last_pos = pos - 1;
        int is_path = 0;
        while (last_pos > 0 && cmd[last_pos] != ' ') {
            if (cmd[last_pos] == '/' || cmd[last_pos] == '.')
                is_path = 1;
            last_pos--;
        }
        if (cmd[last_pos] == '/' || cmd[last_pos] == '.')
            is_path = 1;
        
        if (cmd[last_pos] == ' ')
            last_pos++;
        char path[MAX_FILE_NAME];
        strCopy(&cmd[last_pos], path);
        if (!is_path && last_pos == 0)
            complete_cmd(path);
        else
            complete(path);
    }
    else {
        cmd[pos++] = c;
        stream_putchar(c, STDOUT);
    }
}

char *cut_filename(char *path, char **dirname) {
    char *start = path;
    while (*path)
        path++;
    while (*path != '/' && path > start)
        path--;
    if (*path == '/') {
        *path = 0;
        if (!*start)
            *dirname = "/";
        else
            *dirname = start;
        return (path + 1);
    }
    *dirname = ".";
    return start;
}

int is_prefix(char *a, char *b) {
    // tests if a is a prefix of b.
    while (*a) {
        if (*a++ != *b++)
            return 0;
    }
    return 1;
}

void complete_cmd(char *start) {
    char best_prefix[50];
    char *solutions[100];
    int nb_solutions = 0;
    
    for (int i = 0; i < NB_SHELL_CMD; i++) {
        int res = is_prefix(start, shell_commands[i]);
        //printf("test <%s> : %d\n", dirent.name, res);
        if (res) {
            if (!nb_solutions) {
                strCopy(shell_commands[i], best_prefix);
            }
            else {
                char *new = shell_commands[i];
                char *old = best_prefix;
                while (*new && *old && (*new == *old)) {
                    new++;
                    old++;
                }
                *old = 0;
                //printf("new best prefix : %s\n", best_prefix);
            }
            if (nb_solutions < 100) {
                int len = strlen(shell_commands[i]);
                char *sol = malloc(len+1);
                strCopy(shell_commands[i], sol);
                solutions[nb_solutions] = sol;
                nb_solutions++;
            }
        }
    }
    for (int i = 0; i < NB_BUILTIN; i++) {
        int res = is_prefix(start, builtin_commands[i]);
        //printf("test <%s> : %d\n", dirent.name, res);
        if (res) {
            if (!nb_solutions) {
                strCopy(builtin_commands[i], best_prefix);
            }
            else {
                char *new = builtin_commands[i];
                char *old = best_prefix;
                while (*new && *old && (*new == *old)) {
                    new++;
                    old++;
                }
                *old = 0;
                //printf("new best prefix : %s\n", best_prefix);
            }
            if (nb_solutions < 100) {
                int len = strlen(builtin_commands[i]);
                char *sol = malloc(len+1);
                strCopy(builtin_commands[i], sol);
                solutions[nb_solutions] = sol;
                nb_solutions++;
            }
        }
    }    
    char *res = best_prefix;
    if (!nb_solutions)
        return;
    while (*start) {
        start++;
        res++;
    }
    if (nb_solutions > 1 && !*res) {
        // We want to display the options
        stream_putchar('\n', STDOUT);
        char *sol;
        for (int i =0; i < nb_solutions; i++) {
            sol = solutions[i];
            printf("%s   ", sol);
            free(sol);
        }
        printf("\n");
        if (nb_solutions == 100)
            printf("There may be more possibilities.\n");
        new_cmd();
        printf("%s", cmd);
        flush(STDOUT);
        return;
    }
    char c;
    while (*res) {
        c = *res++;
        cmd[pos++] = c;
        stream_putchar(c, STDOUT);
    }
    if (nb_solutions == 1) {
        cmd[pos++] = ' ';
        stream_putchar(' ', STDOUT);
    }
    return;
}

void complete(char *path) {
    char *dirname;
    char *start_name = cut_filename(path, &dirname);
    //printf("Completion search for dirname %s, filename %s\n", dirname, start_name);
    fd_t dir = opendir(dirname);
    if (dir < 0)
        return;
    char best_prefix[MAX_FILE_NAME];
    dirent_t dirent;
    int complete_dir = 0;
    char *solutions[100];
    int nb_solutions = 0;
    while (readdir(dir, &dirent) == 0) {
        int res = is_prefix(start_name, dirent.name);
        //printf("test <%s> : %d\n", dirent.name, res);
        if (res) {
            if (!nb_solutions) {
                strCopy(dirent.name, best_prefix);
                if (dirent.type == DIR)
                    complete_dir = 1;
            }
            else {
                complete_dir = 0;
                char *new = dirent.name;
                char *old = best_prefix;
                while (*new && *old && (*new == *old)) {
                    new++;
                    old++;
                }
                *old = 0;
                //printf("new best prefix : %s\n", best_prefix);
            }
            if (nb_solutions < 100) {
                int len = strlen(dirent.name);
                char *sol = malloc(len+2);
                strCopy(dirent.name, sol);
                if (dirent.type == DIR && !strEqual(dirent.name, "/") && dirent.name[0] != '.') {
                    sol[len] = '/';
                    sol[len+1] = 0;
                }
                solutions[nb_solutions] = sol;

                nb_solutions++;
            }
        }
    }
    //printf("Final decision : %s\n", best_prefix);
    closedir(dir);
    char *res = best_prefix;
    if (!nb_solutions)
        return;
    while (*start_name) {
        start_name++;
        res++;
    }
    if (nb_solutions > 1 && !*res) {
        // We want to display the options
        stream_putchar('\n', STDOUT);
        char *sol;
        for (int i =0; i < nb_solutions; i++) {
            sol = solutions[i];
            char *end =sol;
            while (*end != '/' && *end)
                end++;
            if (*end)
                printf("%bg%s%pbg   ", BLUE, sol);
            else
                printf("%s   ", sol);
            free(sol);
        }
        printf("\n");
        if (nb_solutions == 100)
            printf("There may be more possibilities.\n");
        new_cmd();
        printf("%s", cmd);
        flush(STDOUT);
        return;
    }
    char c;
    while (*res) {
        c = *res++;
        cmd[pos++] = c;
        stream_putchar(c, STDOUT);
    }
    if (complete_dir) {
        cmd[pos++] = '/';
        stream_putchar('/', STDOUT);
    }
    else if (nb_solutions == 1) {
        cmd[pos++] = ' ';
        stream_putchar(' ', STDOUT);
    }
    return;
}

int main(char *init_cmd) {
    ephemeral = *init_cmd;
    run = 1;
    pos = 0;
    flush(STDOUT);
    memset(cwd, 0, MAX_PATH_NAME);
    getcwd(cwd);
    if (ephemeral) {
        //printf("EPHEMERAL\n");
        exit(exec_cmd(init_cmd));
    }
    strCopy(default_user, user_name);
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
