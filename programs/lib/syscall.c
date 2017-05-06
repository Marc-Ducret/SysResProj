#include "syscall.h"
/* This file contains all the syscalls seen from the user side.
   This means that the user is supposed to have access to it. */

// TODO Déterminer s'il faut indiquer la modification de tous les registres ?
// Check that no casts are needed (oflags_t, ..)


pid_t exec(char *cmd) {
    pid_t res;
    int errno;
    
    asm volatile("\
        movl $3, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (cmd)
        : "%ebx", "esi", "edi"
        );
    return res;
}

pid_t wait(int *status) {
    int errno;
    int exit_value;
    pid_t child;
    
    asm volatile("\
        movl $5, %%eax \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1 \n \
        movl %%ecx, %2"
        : "=m" (child), "=m" (errno), "=m" (exit_value)
        :
        : "%ebx", "esi", "edi"
        );
    if (child >= 0)
        *status = exit_value;
    return child;
}

void exit(int status) {
    asm volatile("\
        movl $4, %%eax \n \
        movl %0, %%ebx \n \
        int $0x80"
        :
        : "m" (status)
        : "%ebx", "esi", "edi"
        );
    // This syscall should never return.
    //kprintf("WARNING : Returned after exit. Trying again.\n");
    //kexit(status);
}

ssize_t send(int chanid, u8 *buffer, size_t len) {
    ssize_t res;
    int errno;
    
    asm volatile("\
        movl $1, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        movl %4, %%esi \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (chanid), "m" (buffer), "m" (len)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int receive(int chanid, u8 *buffer, size_t len) {    
    ssize_t res;
    int errno;
    
    asm volatile("\
        movl $1, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        movl %4, %%esi \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (chanid), "m" (buffer), "m" (len)
        : "%ebx", "esi", "edi"
        );
    return res;
}

pid_t wait_channel(int chanid) {
    pid_t sender;
    int errno;
        
    asm volatile("\
        movl $7, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (sender), "=m" (errno)
        : "m" (chanid)
        : "%ebx", "esi", "edi"
        );
    return sender;
}

int new_channel(void) {
    int res;
    int errno;
    
    asm volatile("\
                movl $0, %%eax \n \
                int $0x80 \n \
                movl %%eax, %0 \n \
                movl %%ebx, %1"
                : "=m" (res), "=m" (errno)
                :
                : "%ebx", "esi", "edi");
    return res;
}

int free_channel(int chanid) {
    int res;
    int errno;
        
    asm volatile("\
        movl $6, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (chanid)
        : "%ebx", "esi", "edi"
        );
    return res;
}


// File System related Calls

fd_t fopen(char *path, oflags_t flags) {
    fd_t fd;
    int errno;
    
    asm volatile("\
        movl $10, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (fd), "=m" (errno)
        : "m" (path), "m" (flags)
        : "%ebx", "esi", "edi"
        );
    return fd;
}

int close(fd_t fd) {
    int res;
    int errno;
    
    asm volatile("\
        movl $11, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd)
        : "%ebx", "esi", "edi"
        );
    return res;
}
ssize_t read(fd_t fd, void *buffer, size_t length) {
    ssize_t res;
    int errno;
    
    asm volatile("\
        movl $12, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        movl %4, %%esi \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd), "m" (buffer), "m" (length)
        : "%ebx", "esi", "edi"
        );
    return res;
}

ssize_t write(fd_t fd, void *buffer, size_t length) {
    ssize_t res;
    int errno;
    
    asm volatile("\
        movl $13, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        movl %4, %%esi \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd), "m" (buffer), "m" (length)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int seek(fd_t fd, seek_cmd_t seek_command, int offset) {
    int errno;
    int res;
    
    asm volatile("\
        movl $14, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        movl %4, %%esi \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd), "m" (seek_command), "m" (offset)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int mkdir(char *path, u8 mode) {
    int res;
    int errno;
    
    asm volatile("\
        movl $20, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (path), "m" (mode)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int rmdir(char *path) {
    int res;
    int errno;
    
    asm volatile("\
        movl $21, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (path)
        : "%ebx", "esi", "edi"
        );
    return res;
}
int chdir(char *path) {
    int res;
    int errno;
    
    asm volatile("\
        movl $22, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (path)
        : "%ebx", "esi", "edi"
        );
    return res;
}

char *kgetcwd() {
    char *res;
    int errno;
    
    asm volatile("\
        movl $23, %%eax \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        :
        : "%ebx", "esi", "edi");
    return res;
}

fd_t opendir(char *path) {
    fd_t res;
    int errno;
    
    asm volatile("\
        movl $24, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (path)
        : "%ebx", "esi", "edi"
        );
    return res;
}

dirent_t *kreaddir(fd_t fd) {
    dirent_t *res;
    int errno;
    
    asm volatile("\
        movl $25, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int rewinddir(fd_t fd) {
    int res;
    int errno;
    
    asm volatile("\
        movl $26, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int closedir(fd_t fd) {
    int res;
    int errno;
    
    asm volatile("\
        movl $27, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd)
        : "%ebx", "esi", "edi"
        );
    return res;
}

/*
dirent_t *finddir(fd_t dir, char *name);
dirent_t *findfile(fd_t dir, char *name);
dirent_t *findent(fd_t dir, char *name, ftype_t type);
*/

int get_key_event() {
    int res;
    int errno;
    
    asm volatile("\
                movl $40, %%eax \n \
                int $0x80 \n \
                movl %%eax, %0 \n \
                movl %%ebx, %1"
                : "=m" (res), "=m" (errno)
                :
                : "%ebx", "esi", "edi");
    return res;
}
/*
void new_launch() {
    printf("Initial state\n");
    chanid channels[4];
    // A priori superflu
    context_t ctx;
    state* s = picoinit(&ctx);
    log_state(s);

    printf("Forking init\n");
    pid_t init = fork(MAX_PRIORITY);
    printf("We have a new process : %d\n", init);
    log_state(s);

    printf("Asking for a new channel, in r4\n");
    chanid chan1 = new_channel();
    printf("Channel obtenu : %d\n", chan1);
    log_state(s);
    
    printf("Making init wait for a message on the channel\n");
    printf("This should switch to the child process since init is BlockedReading\n");
    
    channels[0] = chan1;
    channels[1] = -1;
    channels[2] = -1;
    channels[3] = -1;
    int res;
    printf("J'ai demande %d \n", chan1);
    receive(channels, &res);
    log_state(s);

    printf("Getting a new channel in r3\n");
    //s->registers->eax = 0;
    //picotransition(s, SYSCALL);
    //s->registers->edx = s->registers->eax;
    chanid chan2 = new_channel();
    log_state(s);

    printf("What about having a child of our own?\n");
    //s->registers->eax = 3;
    //s->registers->ebx = MAX_PRIORITY - 1;
    //picotransition(s, SYSCALL);
    pid_t child2 = fork(MAX_PRIORITY - 1);
    log_state(s);

    printf("Let's wait for him to die!\n");
    //s->registers->eax = 5;
    //picotransition(s, SYSCALL);
    int status;
    int res_wait = wait(&status);
    log_state(s);

    printf("On with the grandchild, which'll send on channel r3\n");
    //s->registers->eax = 1;
    //s->registers->ebx = s->registers->edx;
    //s->registers->ecx = -12;
    //picotransition(s, SYSCALL);
    int res_send = send(chan2, -12);
    log_state(s);

    printf("On with idle, to listen to the grandchild!\n");
    //s->registers->eax = 2;
    //s->registers->ebx = 1; // Little hack, not supposed to now it's gonna be channel one
    //s->registers->ecx = -1;
    //s->registers->edx = -1;
    //s->registers->esi = -1;
    //picotransition(s, SYSCALL);
    channels[0] = 1;
    int dest2;
    int res_recv = receive(channels, &dest2);
    log_state(s);

    printf("Letting the timer tick until we're back to the grandchild\n");
    for (int i = MAX_TIME_SLICES; i >= 0; i--) {
        picotransition(s, TIMER);
    }
    log_state(s);

    printf("Hara-kiri\n");
    //s->registers->eax = 4;
    //s->registers->ebx = 125;
    //picotransition(s, SYSCALL);
    exit(125);
    log_state(s);

    printf("Let's speak to dad!\n");
    //s->registers->eax = 1;
    //s->registers->ebx = s->registers->esi;
    //s->registers->ecx = 42;
    //picotransition(s, SYSCALL);
    send(chan1, 42);
    log_state(s);

    printf("Our job is done, back to dad! (see 42 in r2?)\n");
    //s->registers->eax = 4;
    //s->registers->ebx = 12; // Return value
    //picotransition(s, SYSCALL);
    exit(12);
    log_state(s);

    printf("Let's loot the body of our child (see 12 in r2?)\n");
    //s->registers->eax = 5;
    //spicotransition(s, SYSCALL);

    wait(&status);
    log_state(s);
}
*/
