#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printing.h"
#include "kernel.h"


/* This file contains all the syscalls seen from the user side.
   This means that the user is supposed to ahave access to it. */

// TODO Determiner si il faut sauver les registres avant de mettre les args.

pid_t kfork(priority prio) {
    asm("movl $3, %eax");
    asm volatile("movl %0, %%ebx"::"" (prio));
    
    asm("int $0x80");
    
    pid_t child;
    asm("movl %%ebx, %0":"=rm" (child):);
    
    int err;
    asm("movl %%eax, %0":"=rm" (err):);
    
    if (!err)
        return -1;
    
    return child;
}

pid_t kwait(int *status) {
    asm("movl $5, %eax");
    
    asm("int $0x80");
    
    int res;
    asm("movl %%eax, %0":"=rm" (res):);
    
    if (res) {
        // A Zombie child has been found.
        pid_t child;
        int ret_value;
        asm("movl %%ebx, %0":"=rm" (child):);
        asm("movl %%ecx, %0":"=rm" (ret_value):);
        
        *status = ret_value;
        return child;
    }
    else {
        // No child found.
        return -1;
    }
}

void kexit(int status) {
    asm("movl $4, %eax");
    asm volatile("movl %0, %%ebx"::""((status)));
    
    asm("int $0x80");
    
    // This syscall should never return.
    kprintf("WARNING : Returned after exit. Trying again.\n");
    //kexit(status);
}

// Utilisera-t-on des file descriptors ? TODO
int ksend(chanid channel, int msg) {
    asm("movl $1, %eax");
    asm volatile("movl %0, %%ebx"::"" (channel));
    asm volatile("movl %0, %%ecx"::"" (msg));
    
    asm("int $0x80");
    
    int ret_value;
    asm("movl %%eax, %0":"=rm" (ret_value):);
    
    if (ret_value) {
        return 0;
    }
    else {
        // No such channel or already occupied.
        return -1;  
    }
}

int kreceive(chanid channel[4], int *dest) {
    asm("movl $2, %eax");
    asm("movl %0, %%ebx"::"" (channel[0]));
    asm("movl %0, %%ecx"::"" (channel[1]));
    asm("movl %0, %%edx"::"" (channel[2]));
    asm("movl %0, %%esi"::"" (channel[3]));
    
    asm("int $0x80");
    
    int ret_value;
    asm("movl %%eax, %0":"=rm" (ret_value):);
    
    if (ret_value) {
        int res;
        chanid chan;
        asm("movl %%ebx, %0":"=rm" (chan):);
        asm("movl %%eax, %0":"=rm" (res):);
        
        //TODO On ignore le channel choisi ici ?
        *dest = res;
        return 0;
    }
    else {
        // No valid channel 
        return -1;
    }
}

chanid knew_channel() {
    asm("movl $0, %eax");
    
    asm("int $0x80");
    
    chanid chan;
    asm("movl %%ebx, %0":"=rm" (chan):);
    
    int ret_value;
    asm("movl %%eax, %0":"=rm" (ret_value):);
    
    if (!ret_value)
        return -1;
    
    return chan;
}

void new_launch() {
    kprintf("Initial state\n");
    chanid channels[4];
    // A priori superflu
    registers_t regs;
    state* s = picoinit(&regs);
    log_state(s);
    for (;;){}
    kprintf("Forking init\n");
    //s->registers->eax = 3;
    //s->registers->ebx = MAX_PRIORITY;
    //picotransition(s, SYSCALL);
    pid_t init = kfork(MAX_PRIORITY);
    log_state(s);
    for (;;){}
    kprintf("Asking for a new channel, in r4\n");
    //s->registers->eax = 0;
    //picotransition(s, SYSCALL);
    chanid chan1 = knew_channel();
    //s->registers->esi = s->registers->eax;
    log_state(s);
    for (;;){}
    kprintf("Making init wait for a message on the channel\n");
    kprintf("This should switch to the child process since init is BlockedReading\n");
    //s->registers->eax = 2;
    //s->registers->ebx = -1;
    //s->registers->ecx = -1;
    //s->registers->edx = -1;
    //picotransition(s, SYSCALL);
    channels[0] = chan1;
    channels[1] = -1;
    channels[2] = -1;
    channels[3] = -1;
    int res;
    
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
    for (;;){}
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
    
    return;
}