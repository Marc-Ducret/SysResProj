#include "lib.h"

#define CMD_SIZE 0x200

u8 recv_buff[512];

char cmd[CMD_SIZE];
int pos;
int run;

void new_cmd() {
    printf("> ");
    flush(STDOUT);
}

void exec_cmd() {
    if(     strEqual(cmd, "help")) printf("Available commands: help, exit, int, test\n");
    else if(strEqual(cmd, "exit")) run = 0;
    else if(strEqual(cmd, "test")) {
        for(int i = 0; i < 100; i ++)
            printf("%d\n", i);
    } else if(strEqual(cmd, "int")) {
        asm volatile ("int $0x03");
        asm volatile ("int $0x03");
    } else if(cmd[0] == '/') {
        exec(cmd, -1, STDOUT);
        if(errno)
            printf("Error: %s\n", strerror(errno));
    }
    else printf("Unknown command (%s)\n", cmd);
    while(pos > 0)
        cmd[--pos] = 0;
    new_cmd();
}

void key_typed(u8 c) {
    if(c == '\n' || pos == CMD_SIZE-1) {
        stream_putchar(c, STDOUT);
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
