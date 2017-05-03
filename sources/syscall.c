#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printing.h"
#include "kernel.h"
#include "syscall.h"
#include "int.h"
/* This file contains all the syscalls seen from the user side.
   This means that the user is supposed to have access to it. */

// TODO Déterminer s'il faut indiquer la modification de tous les registres ?
// Check that no casts are needed (oflags_t, ..)
pid_t kfork(priority prio) {
    kprintf("Requested a fork, with priority %d\n", prio);
    pid_t child;
    int errno;
    
    asm volatile("\
        movl $3, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (child), "=m" (errno)
        : "m" (prio)
        : "%ebx", "esi", "edi"
        );
    kprintf("Fork returned with child %d, and errno %d\n", child, errno);
    return child;
}

pid_t kwait(int *status) {
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

void kexit(int status) {
    asm volatile("\
        movl $4, %%eax \n \
        movl %0, %%ebx \n \
        int $0x80"
        :
        : "m" (status)
        : "%ebx", "esi", "edi"
        );
    // This syscall should never return.
    kprintf("WARNING : Returned after exit. Trying again.\n");
    //kexit(status);
}

ssize_t ksend(int chanid, u8 *buffer, size_t len) {
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

int kreceive(int chanid, u8 *buffer, size_t len) {    
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

pid_t kwait_channel(int chanid) {
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

int knew_channel(void) {
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

int kfree_channel(int chanid) {
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

fd_t kfopen(char *path, oflags_t flags) {
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

int kclose(fd_t fd) {
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
ssize_t kread(fd_t fd, void *buffer, size_t length) {
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

ssize_t kwrite(fd_t fd, void *buffer, size_t length) {
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

int kseek(fd_t fd, seek_cmd_t seek_command, int offset) {
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

int kmkdir(char *path, u8 mode) {
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

int krmdir(char *path) {
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
int kchdir(char *path) {
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

fd_t kopendir(char *path) {
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

int krewinddir(fd_t fd) {
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

int kclosedir(fd_t fd) {
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

int kget_key_event() {
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
    kprintf("Initial state\n");
    chanid channels[4];
    // A priori superflu
    context_t ctx;
    state* s = picoinit(&ctx);
    log_state(s);

    kprintf("Forking init\n");
    pid_t init = kfork(MAX_PRIORITY);
    kprintf("We have a new process : %d\n", init);
    log_state(s);

    kprintf("Asking for a new channel, in r4\n");
    chanid chan1 = knew_channel();
    kprintf("Channel obtenu : %d\n", chan1);
    log_state(s);
    
    kprintf("Making init wait for a message on the channel\n");
    kprintf("This should switch to the child process since init is BlockedReading\n");
    
    channels[0] = chan1;
    channels[1] = -1;
    channels[2] = -1;
    channels[3] = -1;
    int res;
    kprintf("J'ai demande %d \n", chan1);
    kreceive(channels, &res);
    log_state(s);

    kprintf("Getting a new channel in r3\n");
    //s->registers->eax = 0;
    //picotransition(s, SYSCALL);
    //s->registers->edx = s->registers->eax;
    chanid chan2 = knew_channel();
    log_state(s);

    kprintf("What about having a child of our own?\n");
    //s->registers->eax = 3;
    //s->registers->ebx = MAX_PRIORITY - 1;
    //picotransition(s, SYSCALL);
    pid_t child2 = kfork(MAX_PRIORITY - 1);
    log_state(s);

    kprintf("Let's wait for him to die!\n");
    //s->registers->eax = 5;
    //picotransition(s, SYSCALL);
    int status;
    int res_wait = kwait(&status);
    log_state(s);

    kprintf("On with the grandchild, which'll send on channel r3\n");
    //s->registers->eax = 1;
    //s->registers->ebx = s->registers->edx;
    //s->registers->ecx = -12;
    //picotransition(s, SYSCALL);
    int res_send = ksend(chan2, -12);
    log_state(s);

    kprintf("On with idle, to listen to the grandchild!\n");
    //s->registers->eax = 2;
    //s->registers->ebx = 1; // Little hack, not supposed to know it's gonna be channel one
    //s->registers->ecx = -1;
    //s->registers->edx = -1;
    //s->registers->esi = -1;
    //picotransition(s, SYSCALL);
    channels[0] = 1;
    int dest2;
    int res_recv = kreceive(channels, &dest2);
    log_state(s);

    kprintf("Letting the timer tick until we're back to the grandchild\n");
    for (int i = MAX_TIME_SLICES; i >= 0; i--) {
        picotransition(s, TIMER);
    }
    log_state(s);

    kprintf("Hara-kiri\n");
    //s->registers->eax = 4;
    //s->registers->ebx = 125;
    //picotransition(s, SYSCALL);
    kexit(125);
    log_state(s);

    kprintf("Let's speak to dad!\n");
    //s->registers->eax = 1;
    //s->registers->ebx = s->registers->esi;
    //s->registers->ecx = 42;
    //picotransition(s, SYSCALL);
    ksend(chan1, 42);
    log_state(s);

    kprintf("Our job is done, back to dad! (see 42 in r2?)\n");
    //s->registers->eax = 4;
    //s->registers->ebx = 12; // Return value
    //picotransition(s, SYSCALL);
    kexit(12);
    log_state(s);

    kprintf("Let's loot the body of our child (see 12 in r2?)\n");
    //s->registers->eax = 5;
    //spicotransition(s, SYSCALL);

    kwait(&status);
    log_state(s);
}
*/