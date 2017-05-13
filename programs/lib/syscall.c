#include "syscall.h"
/* This file contains all the syscalls seen from the user side.
   This means that the user is supposed to have access to it. */

// TODO Déterminer s'il faut indiquer la modification de tous les registres ?
// Check that no casts are needed (oflags_t, ..)

pid_t exec(char *file, char *args, int chin, int chout) {
    pid_t res;
    
    asm volatile("\
        movl $3, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        movl %4, %%esi \n \
        movl %5, %%edi \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (file), "m" (args), "m" (chin), "m" (chout)
        : "%ebx", "esi", "edi"
        );
    return res;
}

pid_t wait(int *status) {
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
    // Flushes all channels and leaves.
    flush(STDOUT);
    
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

ssize_t send(int chanid, void *buffer, size_t len) {
    ssize_t res;
    
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

int receive(int chanid, void *buffer, size_t len) {    
    ssize_t res;
    
    asm volatile("\
        movl $2, %%eax \n \
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

ssize_t wait_channel(int chanid, int write) {
    ssize_t sender;
    
    asm volatile("\
        movl $7, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (sender), "=m" (errno)
        : "m" (chanid), "m" (write)
        : "%ebx", "esi", "edi"
        );
    return sender;
}

int new_channel(void) {
    int res;
    
    asm volatile("\
                movl $0, %%eax \n \
                int $0x80 \n \
                movl %%eax, %0 \n \
                movl %%ebx, %1"
                : "=m" (res), "=m" (errno)
                :
                : "%ebx", "esi", "edi");
    if (res >= 0)
        create_channel_stream(res);
    return res;
}

int free_channel(int chanid) {
    int res;
    
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

int sleep(int time) {
    int res;
    
    asm volatile("\
        movl $8, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (time)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int pinfo(pid_t pid, process_info_t *data) {
    int res;
    
    asm volatile("\
        movl $9, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (pid), "m" (data)
        : "%ebx", "esi", "edi"
        );
    return res;
}

// File System related Calls

fd_t fopen(char *path, oflags_t flags) {
    fd_t fd;
    
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

int remove(char *path) {
    int res;
    
    asm volatile("\
        movl $15, %%eax \n \
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

int fcopy(char *src, char *dest) {
    int res;
    
    asm volatile("\
        movl $16, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (src), "m" (dest)
        : "%ebx", "esi", "edi"
        );
    return res;
}

int mkdir(char *path, u8 mode) {
    int res;
    
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

int getcwd(char *buffer) {
    int res;
    
    asm volatile("\
        movl $23, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (buffer)
        : "%ebx", "esi", "edi");
    return res;
}

fd_t opendir(char *path) {
    fd_t res;
    
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

int readdir(fd_t fd, dirent_t *dirent) {
    int res;
    
    asm volatile("\
        movl $25, %%eax \n \
        movl %2, %%ebx \n \
        movl %3, %%ecx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (fd), "m" (dirent)
        : "%ebx", "esi", "edi"
        );
    return res;
}
void _test() {
    int res;
    asm volatile("\
        movl $28, %%eax \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        :
        : "%ebx", "esi", "edi"
        );
    res = res;
}
int rewinddir(fd_t fd) {
    int res;
    
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

int gettimeofday(rtc_time_t *t) {
    int res;
    
    asm volatile("\
                movl $41, %%eax \n \
                movl %2, %%ebx \n \
                int $0x80 \n \
                movl %%eax, %0 \n \
                movl %%ebx, %1"
                : "=m" (res), "=m" (errno)
                : "m" (t)
                : "%ebx", "esi", "edi");
    return res;
}

int kill(pid_t pid) {
    int res;
    
    asm volatile("\
        movl $42, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (pid)
        : "%ebx", "esi", "edi"
        );
    return res;
}

void *resize_heap(int delta_size) {
    void *res;
    
    asm volatile("\
        movl $43, %%eax \n \
        movl %2, %%ebx \n \
        int $0x80 \n \
        movl %%eax, %0 \n \
        movl %%ebx, %1"
        : "=m" (res), "=m" (errno)
        : "m" (delta_size)
        : "%ebx", "esi", "edi"
        );
    return res;
}
