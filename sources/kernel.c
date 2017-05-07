#include "kernel.h"

syscall_fun_t syscall_fun[NUM_SYSCALLS];

list* malloc_list() {
    static int base = 0;
    int i;
    
    for (i=base; i < MAX_SIZE_LIST; i++){
        if (list_memory[i].free == 0) {
            base = i + 1;
            if (base == MAX_SIZE_LIST)
                base = 0;
            
            list_memory[i].free = 1;
            return &(list_memory[i].elt);
        }
    }
    
    if (base != 0)
        return malloc_list();
    kprintf("Memory Error");
    return 0;
}

void free_list(list* l) {
    int i;
    for (i=0; i < MAX_SIZE_LIST; i++){
        if (&(list_memory[i].elt) == l) {
            list_memory[i].free = 0;
            return;
        }
    }
    kprintf("Invalid argument : no such list element to free.");
}

c_list* malloc_c_list() {
    static int base = 0;
    int i;
    
    for (i=base; i < MAX_SIZE_C_LIST; i++){
        if (c_list_memory[i].free == 0) {
            base = i + 1;
            if (base == MAX_SIZE_C_LIST)
                base = 0;
            
            c_list_memory[i].free = 1;
            return &(c_list_memory[i].elt);
        }
    }
    
    if (base != 0)
        return malloc_c_list();
    kprintf("Memory Error");
    return 0;
}

void free_c_list(c_list* l) {
    int i;
    for (i=0; i < MAX_SIZE_C_LIST; i++){
        if (&(c_list_memory[i].elt) == l) {
            c_list_memory[i].free = 0;
            return;
        }
    }
    kprintf("Invalid argument : no such c_list element to free.");
}

list* add(int hd, list* tl) {
    list *res = malloc_list();
    res->hd = hd;
    res->tl = tl;

    return res;
}

list* add_end(int hd, list* l) {
    if(l == NULL) {
        list *res = malloc_list();
        res->hd = hd;
        res->tl = NULL; 
        return res;
    }
    l->tl = add_end(hd, l->tl);
    return l;
}

list *filter(list *l, int elt) {
    //On ne le fait pas en place !
    if (l == NULL) {
        return NULL;
    }

    list *temp = filter(l->tl, elt);

    if (l->hd == elt) {
        free_list(l); // Ni de f
        return temp;
    }

    l->tl = temp;
    return l;
}

volatile u32 user_esp;
volatile page_directory_t *user_pd;
volatile multiboot_info_t *multiboot_info;

state *get_global_state() {
    return &global_state;
}

void copy_context(context_t *src, context_t *dst) {
    memcpy(dst, src, sizeof(context_t));
}

int start_process(int parent, char* cmd, int chin, int chout) {
    pid_t pid = 0;
    while(global_state.processes[pid].state != FREE) {
        if(++pid == NUM_PROCESSES) {
            errno = EMPROC;
            return -1;
        }
    }
    
    process *p = &global_state.processes[pid];
    p->parent_id = parent;
    p->state = RUNNABLE;
    p->slices_left = 0;
    
    p->channels[0].chanid = chin;
    p->channels[0].write  = 0;
    p->channels[0].read   = 1;
    p->channels[1].chanid = chout;
    p->channels[1].write  = 1;
    p->channels[1].read   = 0;
    
    page_directory_t *pd = init_user_page_dir(cmd, get_identity());
    if (pd == NULL) {
        return -1;
    }
    p->page_directory = pd;
    copy_context(global_state.ctx, &p->saved_context);
    p->saved_context.stack.eip = USER_CODE_VIRTUAL;
    p->saved_context.regs.esp = USER_STACK_VIRTUAL + 0x1000 - sizeof(context_t) - 0x8 + 0x2C;
    global_state.runqueues[MAX_PRIORITY] = add(pid, global_state.runqueues[MAX_PRIORITY]);
    return pid;
}

int _exit(state *s) {
    pid_t pid = s->curr_pid;
    s->processes[pid].state = ZOMBIE;
    pid_t pere = s->processes[pid].parent_id;

    //On trouve les processus fils de celui ci.
    pid_t j;
    for (j = 0; j < NUM_PROCESSES; j++) {
        if (s->processes[j].parent_id == pid) {
            s->processes[j].parent_id = 1; // TODO No hardcode of init process
        }
    }

    //On cherche aussi le processus parent de celui-ci, s'il est en wait.
    if (s->processes[pere].state == WAITING) {
        //On fait la même chose que dans wait.
        s->processes[pere].state = RUNNABLE;
        s->processes[pid].state = FREE;
        registers_t *regs = &(s->processes[pere].saved_context.regs);
        regs->eax = pid;
        regs->ebx = 0;
        regs->ecx = s->ctx->regs.ebx;
    }

    //On enlève le processus de sa file
    s->runqueues[s->curr_priority] = filter(s->runqueues[s->curr_priority], pid);
    
    return 1; // Need to reorder
}

int _wait(state *s) {
    pid_t pid = s->curr_pid;
    int fils = 0;
    // Searchs for a Zombie child
    pid_t j;
    for (j = 0; j < NUM_PROCESSES; j++) {
        if (s->processes[j].parent_id == pid) {
            fils = 1;
            if (s->processes[j].state == ZOMBIE) {
                s->ctx->regs.eax = j;
                s->ctx->regs.ebx = 0;
                s->ctx->regs.ecx = s->processes[j].saved_context.regs.ebx;
                s->processes[j].state = FREE;
                return 0;
            }
        }
    }

    if (fils) {
        s->processes[pid].state = WAITING;
        return 1;
    }

    s->ctx->regs.eax = -1;
    s->ctx->regs.ebx = ECHILD;
    return 0;
}

int _new_channel(state *s) {
    int chanid = new_channel(s->processes[s->curr_pid].channels);
    s->ctx->regs.eax = chanid;
    s->ctx->regs.ebx = errno;
    return 0; // No reorder
}

int _free_channel(state *s) {
    int res = free_channel(s->ctx->regs.ebx, s->processes[s->curr_pid].channels);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0; // No reorder
}

int _send(state *s) {
    ssize_t len = send(s);
    s->ctx->regs.eax = len;
    s->ctx->regs.ebx = errno;
    return (len >= 0); // Need to reorder if no error
}

int _wait_channel(state *s) {
    pid_t sender = wait_channel(s);
    s->ctx->regs.eax = sender;
    s->ctx->regs.ebx = errno;
    // Need to reorder only if there was no error and no sender.
    return (sender == -1) && (errno == ECLEAN);
}

int _receive(state *s) {
    ssize_t len = receive(s);
    s->ctx->regs.eax = len;
    s->ctx->regs.ebx = errno;
    return 0; // No need to reorder
}

int _fopen(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    oflags_t flags = (oflags_t) s->ctx->regs.ecx;
    fd_t res = -1;
    if (check_address(path, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = fopen(path, flags);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _close(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    int res = close(fd);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _read(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    void *buffer = (void *) s->ctx->regs.ecx;
    size_t length = (size_t) s->ctx->regs.esi;
    ssize_t res = -1;
    if (check_address(buffer, 1, 1, s->processes[s->curr_pid].page_directory) == 0)
        res = read(fd, buffer, length);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _write(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    void *buffer = (void *) s->ctx->regs.ecx;
    size_t length = (size_t) s->ctx->regs.esi;
    ssize_t res = -1;
    if (check_address(buffer, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = write(fd, buffer, length);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _seek(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    seek_cmd_t seek_command = (seek_cmd_t) s->ctx->regs.ecx;
    int offset = s->ctx->regs.esi;
    int res = seek(fd, seek_command, offset);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _mkdir(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    u8 mode = (u8) s->ctx->regs.ecx;
    int res = -1;
    if (check_address(path, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = mkdir(path, mode);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _rmdir(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    int res = -1;
    if (check_address(path, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = rmdir(path);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _chdir(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    int res = -1;
    if (check_address(path, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = chdir(path);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _getcwd(state *s) {
    char *res = getcwd();
    // TODO Allocation in the user side ?
    res = res;
    s->ctx->regs.eax = (int) NULL;
    s->ctx->regs.ebx = EXDEV;
    return 0;
}

int _opendir(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    fd_t res = -1;
    if (check_address(path, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = opendir(path);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _readdir(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    dirent_t *res = readdir(fd); // TODO Reduce the information ? And allocation !
    s->ctx->regs.eax = (int) res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _rewinddir(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    int res = rewinddir(fd);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _closedir(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    int res = closedir(fd);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _get_key_event(state *s) {
    if(s->curr_pid == s->focus) {
        s->ctx->regs.eax = nextKeyEvent();
    }
    else {
        s->ctx->regs.eax = -1;
        s->ctx->regs.ebx = ENOFOCUS;
    }
    return 0;
}

int _exec(state *s) {
    char *cmd = (char *) s->ctx->regs.ebx;
    int chin  = (int)s->ctx->regs.ecx < 0 ? -1 : s->processes[s->curr_pid].channels[s->ctx->regs.ecx].chanid;
    int chout = (int)s->ctx->regs.edx < 0 ? -1 : s->processes[s->curr_pid].channels[s->ctx->regs.edx].chanid;
    // TODO check validity of channels ! (rights and existence)
    pid_t res = -1;
    if (check_address(cmd, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = start_process(s->curr_pid, cmd, chin, chout);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _default_syscall(state *s) {
    s->ctx->regs.eax = -1;
    s->ctx->regs.ebx = ENOSYS;
    return 0;
}

void reorder(state *s) {
    //On reelit un processus
    pid_t next_pid;
    priority p;
    list *rq;
    for (p = MAX_PRIORITY; p >= 0; p--) {
        rq = s->runqueues[p];
        while (rq != NULL) {
            if (s->processes[rq->hd].state == RUNNABLE) {//&& s->processes[rq->hd].slices_left > 0 ?
                next_pid = rq->hd;
                s->processes[next_pid].slices_left = MAX_TIME_SLICES;
                s->curr_pid = next_pid;
                s->curr_priority = p;
                user_pd = global_state.processes[next_pid].page_directory;
                user_esp = global_state.processes[next_pid].saved_context.regs.esp - 0x2C;
                return;
            }
            rq = rq->tl;
        }

    }
    kprintf("No process to run....");
    asm("hlt");
    return;
}

void picotransition(state *s, event ev) {
    if(s->curr_pid >= 0) copy_context(s->ctx, &(s->processes[s->curr_pid].saved_context));
    int reorder_req = 0;
    if (ev == SYSCALL) {
        int id = s->ctx->regs.eax;
        if (id >= 0 && id < NUM_SYSCALLS) {
            reorder_req = (syscall_fun[id])(s);
        }
        else {
            s->ctx->regs.eax = -1;
            s->ctx->regs.ebx = ENOSYS;
        }
    } else {
        if(s->curr_pid >= 0) {
            if (--s->processes[s->curr_pid].slices_left <= 0) {
                s->processes[s->curr_pid].slices_left = 0;
                reorder_req = 1;
                //On remet le processus a la fin
                priority p = s->curr_priority;
                pid_t id = s->curr_pid;
                s->runqueues[p] = add_end(id, filter(s->runqueues[p], id));
            }
        } else {
            reorder_req = 1;
        }
    }
    
    //user_esp = global_state.processes[s->curr_pid].saved_context.regs.esp - 0x2C; TODO ??
    if (reorder_req) {
        copy_context(s->ctx, &(s->processes[s->curr_pid].saved_context)); // TODO remove redundant saves ?
        reorder(s);
    }
}

void focus_next_process() {
    do
        global_state.focus = (global_state.focus + 1) % NUM_PROCESSES;
    while(global_state.processes[global_state.focus].state == FREE);
    if(global_state.processes[global_state.focus].state != FREE) {
        kprintf("You are looking at a blocked processus. Thus, it isn't able to display its screen, because it isn't updated !\n");
    }
}

void picosyscall(context_t *ctx) {
    // Calls the picotransition with current registers pointing regs.
    global_state.ctx = ctx;
    picotransition(&global_state, SYSCALL);
}

u32 did_init = 0;

void picotimer(context_t *ctx) {
    // Calls the picotransition with current registers pointing regs.
    global_state.ctx = ctx;
    if(!did_init) {
        did_init = 1;
        picoinit();
        return;
    }
    if(global_state.curr_pid == global_state.focus) memcpy((void*) 0xB8000, (void*) USER_SCREEN_VIRTUAL, 0x1000);
    picotransition(&global_state, TIMER);
}

void load_context(context_t ctx) {
    copy_context(&global_state.processes[global_state.curr_pid].saved_context, &ctx);
}

void init_syscalls_table(void) {
    for (int i = 0; i < NUM_SYSCALLS; i++) {
        syscall_fun[i] = _default_syscall;
    }
    
    syscall_fun[0] = _new_channel;
    syscall_fun[1] = _send;
    syscall_fun[2] = _receive;
    syscall_fun[3] = _exec;
    syscall_fun[4] = _exit;
    syscall_fun[5] = _wait;
    syscall_fun[6] = _free_channel;
    syscall_fun[7] = _wait_channel;
    
    syscall_fun[10] = _fopen;
    syscall_fun[11] = _close;
    syscall_fun[12] = _read;
    syscall_fun[13] = _write;
    syscall_fun[14] = _seek;
    
    syscall_fun[20] = _mkdir;
    syscall_fun[21] = _rmdir;
    syscall_fun[22] = _chdir;
    syscall_fun[23] = _getcwd;
    syscall_fun[24] = _opendir;
    syscall_fun[25] = _readdir;
    syscall_fun[26] = _rewinddir;
    syscall_fun[27] = _closedir;
    
    syscall_fun[40] = _get_key_event;
}

state *picoinit() {
    init_syscalls_table();
    state *s = &global_state;
    s->curr_pid = -1;
    s->curr_priority = MAX_PRIORITY;
    
    int i;

    for (i = 0; i < NUM_PROCESSES; i++) {
        s->processes[i].parent_id = 0;
        s->processes[i].state = FREE;
        s->processes[i].slices_left = 0;
        for (int j = 0; j < NUM_CHANNELS_PROC; j++) {
            s->processes[i].channels[j].chanid = -1;
        }
    }

    for (i = 0; i <= MAX_PRIORITY; i++) {
        s->runqueues[i] = NULL;
    }
    start_process(0, "/console.bin /shell.bin", -1, -1);
    reorder(s);
    kprintf("Init kernel\n");
    return s;
}

/*
char* process_to_str(process p) {
    char* state = malloc(100 * sizeof(char));
    switch (p.state.state) {
        case FREE:
            sprintf(state, "Free");
            break;

        case BLOCKEDWRITING:
            sprintf(state, "BlockedWriting (on chan %d)", p.state.ch);
            break;

        case BLOCKEDREADING:
            sprintf(state, "BlockedReading");
            break;

        case WAITING:
            sprintf(state, "Waiting");
            break;

        case RUNNABLE:
            sprintf(state, "Runnable");
            break;

        case ZOMBIE:
            sprintf(state, "Zombie");
            break;
    }

    return state;
}

char* channel_to_str(channel_state c) {
    char* state = malloc(100 * sizeof(char));
    switch (c.state) {
        case UNUSED:
            sprintf(state, "Unused");
            break;

        case SENDER:
            sprintf(state, "%d is sending the value %d", c.s_pid, c.s_value);
            break;

        case RECEIVER:
            sprintf(state, "Receivers");
            break;
    }
    return state;
}
*/

void log_state(state* s) {
    kprintf("Current process is %d (priority %d, %d slices left)\n",
        s->curr_pid, s->curr_priority, s->processes[s->curr_pid].slices_left);
    
    registers_t *regs = &(s->processes[s->curr_pid].saved_context.regs);
    
    kprintf("Registers are: r0=%d, r1=%d, r2=%d, r3=%d, r4=%d\n",
        regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi);

    kprintf("\nRunqueues:\n");
    for (priority prio = MAX_PRIORITY; prio >= 0; prio--) {
        list* q = s->runqueues[prio];
        if (q != NULL) {
            kprintf("Priority %d:\n", prio);
            kprintf("%d(%d)\n", q->hd, s->processes[q->hd].state);
            for (list *curr = q->tl; curr != NULL; curr = curr->tl) {
                kprintf("%d(%d)\n", curr->hd, s->processes[curr->hd].state);
            }
        }
    }
    kprintf("\n");
}
