#ifndef KERNEL_H
#define KERNEL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "int.h"
#include "context.h"
#include "printing.h"
#include "lib.h"
#include "memory.h"
#include "paging.h"
#include "fs_call.h"
#include "keyboard.h"
#include "timer.h"

#define MAX_SIZE_LIST 100
#define MAX_SIZE_C_LIST 100
#define MAX_PRIORITY 15
#define NUM_PROCESSES 100
#define NUM_CHANNELS_PROC 100
#define NUM_REGISTERS 5
#define NUM_HANDLES 32
#define NUM_SYSCALLS 100

#define MAX_TIME_SLICES 100
#define TIMER_SLICES 30
#define SYSCALL_SLICES 10

#define INIT_PROCESS 0

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
    FREE, BLOCKEDWRITING, BLOCKEDREADING, WAITING, RUNNABLE, ZOMBIE, SLEEPING
} p_state;

typedef struct channel_state 
{
    int chanid;
    int read;
    int write;
} channel_state_t;

typedef struct {
    pid_t parent_id;
    p_state state;
    int slices_left;
    context_t saved_context;
    page_directory_t *page_directory;
    channel_state_t channels[NUM_CHANNELS_PROC];
    fd_t cwd;
    char name[256];
    void *heap_pointer;
} process;

typedef struct {
    pid_t pid;
    pid_t parent;
    char name[256];
    p_state state;
} process_info_t;

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

typedef struct  {
    pid_t curr_pid;
    pid_t focus;
    priority curr_priority;
    context_t *ctx;
    process processes[NUM_PROCESSES];
    list* runqueues[MAX_PRIORITY+1];
    c_list *sleeping;
} state;

typedef enum {
    TIMER, SYSCALL 
} event;

typedef int (*syscall_fun_t)(state* s);

state global_state;

void launch();
state *picoinit();
void focus_next_process();
void picosyscall(context_t *);
void picotimer(context_t *);
void picotransition(state *s, event ev);;
void log_state(state *s);
state *get_global_state();
void kill_process(pid_t pid);
void reorder(state *s);
int get_pinfo(pid_t pid, process_info_t *data);
volatile int no_process;
#include "channel.h"
#endif /* KERNEL_H */

