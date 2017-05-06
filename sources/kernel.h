#ifndef KERNEL_H
#define KERNEL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "int.h"
#include "context.h"
#include "multiboot.h"
#include "printing.h"
#include "kernel.h"
#include "lib.h"
#include "memory.h"
#include "paging.h"
#include "fs_call.h"
#include "keyboard.h"

#define MAX_SIZE_LIST 100
#define MAX_SIZE_C_LIST 100
#define MAX_TIME_SLICES 5
#define MAX_PRIORITY 15
#define NUM_PROCESSES 32
#define NUM_CHANNELS 10
#define NUM_REGISTERS 5
#define NUM_HANDLES 32
#define NUM_SYSCALLS 100

typedef int pid_t;
typedef int chanid;
typedef int value;
typedef int priority;

typedef struct list list;
struct list {
    int hd;
    list *tl;
};

struct elt_list {
    int free;
    list elt;
};

struct elt_list list_memory[MAX_SIZE_LIST];

typedef enum p_state 
{
    FREE, BLOCKEDWRITING, BLOCKEDREADING, WAITING, RUNNABLE, ZOMBIE
} p_state;

typedef struct process_state
{
    p_state state;
    list *ch_list;
    chanid ch;
} process_state;

typedef struct process process;
struct process {
    pid_t parent_id;
    process_state state;
    int slices_left;
    context_t saved_context;
    page_directory_t *page_directory;
    char *name;
};

typedef enum c_state c_state;
enum c_state {
    UNUSED, SENDER, RECEIVER
};

typedef struct c_list c_list;
struct c_list {
    pid_t pid;
    priority priority;
    c_list *tl;
};

struct elt_c_list {
    int free;
    c_list elt;
};

struct elt_c_list c_list_memory[MAX_SIZE_C_LIST];

typedef struct channel_state 
{
    int chanid;
    int read;
    int write;
} channel_state;

typedef struct state state;
struct state {
    pid_t curr_pid;
    pid_t focus;
    priority curr_priority;
    context_t *ctx;
    process processes[NUM_PROCESSES];
    list* runqueues[MAX_PRIORITY+1];
};

typedef enum event 
{ TIMER, SYSCALL } event;

typedef int (*syscall_fun_t)(state* s);

state global_state;
volatile multiboot_info_t *multiboot_info;

void launch();
state *picoinit();
void focus_next_process();
void picosyscall(context_t *);
void picotimer(context_t *);
void picotransition(state *s, event ev);;
void log_state(state *s);
state *get_global_state();

#include "channel.h"
#endif /* KERNEL_H */

