#include "kernel.h"

static const int CONSTANTE = 42;
//Variable que dans ce fichier la
//Dans une fonction, elle n'est definie qu'une fois
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

c_list* get_recv(state *s, chanid i) {
    c_list* res = s->channels[i].recvs;
    s->channels[i].recvs = res->tl;
    return res;
}

c_list* remove_recv(pid_t r, c_list* l) {
    //On ne le fait pas en place !
    if (l == NULL) {
        return NULL;
    }

    c_list *temp = remove_recv(r, l->tl);

    if (l->pid == r) {
        free_c_list(l);
        return temp;
    }

    l->tl = temp;
    return l;
}

void release_recv(state *s, pid_t r, list *ch_list) {
    while (ch_list != NULL) {
        s->channels[ch_list->hd].recvs = remove_recv(r, s->channels[ch_list->hd].recvs);
        if (s->channels[ch_list->hd].recvs == NULL) {
            s->channels[ch_list->hd].state = UNUSED;
        }
        ch_list = ch_list->tl;
    }
}


c_list *add_recv(pid_t i, priority p, c_list* l) {
    if (l == NULL || p > l->priority) {
        c_list *res = malloc_c_list();
        res->pid = i;
        res->priority = p;
        res->tl = l;

        return res;
    }

    l->tl = add_recv(i, p, l->tl);
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

syscall_t decode(state *s) {
    syscall_t res;

    switch (s->ctx->regs.eax) {
    case 0:
        res.t = NEWCHANNEL;
        break;

    case 1:
        res.t = SEND;
        res.ch_send = s->ctx->regs.ebx;
        res.val_send = s->ctx->regs.ecx;
        break;

    case 2:
        res.t = RECV;
        res.ch_list = add(s->ctx->regs.ebx,
                          add(s->ctx->regs.ecx,
                              add(s->ctx->regs.edx,
                                  add(s->ctx->regs.esi,
                                      NULL))));
        break;

    case 3: //USELESS
        res.t = FORK;
        res.priority = s->ctx->regs.ebx;
        res.v2 = s->ctx->regs.ecx;
        res.v3 = s->ctx->regs.edx;
        res.v4 = s->ctx->regs.esi;
        break;

    case 4:
        res.t = EXIT;
        break;

    case 5:
        res.t = WAIT;
        break;

    case 40:
        res.t = GET_KEY_EVENT;
        break;

    default:
        res.t = INVALID;
        break;
    }
    return res;
}

/*void picofork(state *s, priority nprio, value v2, value v3, value v4) {
    if (s->curr_priority < nprio) {
        s->ctx->regs.eax = 0;
        return;
    }
    //kprintf("Hi ! I have here : prio %d, v2 %d, v3 %d, v4 %d\n", nprio, v2, v3, v4);
    
    //Good priority
    //Finds the new process
    int i;
    for (i = 0; i < NUM_PROCESSES; i++) {
        if (s->processes[i].state.state == FREE) {
            //Found a free process
            s->ctx->regs.eax = 1;
            s->ctx->regs.ebx = i;
            
            //Creating the new process
            process *new_p = &(s->processes[i]);
            new_p->parent_id = s->curr_pid;
            new_p->slices_left = MAX_TIME_SLICES;
            registers_t *context = &(new_p->saved_context); //VERIFIER
            context->eax = 2;
            context->ebx = s->curr_pid;
            context->ecx = v2;
            context->edx = v3;
            context->esi = v4;
            
            new_p->state.state = RUNNABLE;
            s->runqueues[nprio] = add(i, s->runqueues[nprio]);
            
            return;
        }
    }

    //No free process
    s->ctx->regs.eax = 0;
    return;
}*/

int _exit(state *s) {
    pid_t pid = s->curr_pid;
    s->processes[pid].state.state = ZOMBIE;
    pid_t pere = s->processes[pid].parent_id;

    //On trouve les processus fils de celui ci.
    pid_t j;
    for (j = 0; j < NUM_PROCESSES; j++) {
        if (s->processes[j].parent_id == pid) {
            s->processes[j].parent_id = 1; // TODO No hardcode of init process
        }
    }

    //On cherche aussi le processus parent de celui-ci, s'il est en wait.
    if (s->processes[pere].state.state == WAITING) {
        //On fait la même chose que dans wait.
        s->processes[pere].state.state = RUNNABLE;
        s->processes[pid].state.state = FREE;
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
            if (s->processes[j].state.state == ZOMBIE) {
                s->ctx->regs.eax = j;
                s->ctx->regs.ebx = 0;
                s->ctx->regs.ecx = s->processes[j].saved_context.regs.ebx;
                s->processes[j].state.state = FREE;
                return 0;
            }
        }
    }

    if (fils) {
        s->processes[pid].state.state = WAITING;
        return 1;
    }

    s->ctx->regs.eax = -1;
    s->ctx->regs.ebx = ECHILD;
    return 0;
}

int _new_channel(state *s) {
    int chanid = new_channel();
    s->ctx->regs.eax = chanid;
    s->ctx->regs.ebx = errno;
    return 0; // No reorder
}

int _free_channel(state *s) {
    int res = free_channel(s->ctx->regs.ebx);
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
    fd_t res = fopen(path, flags);
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
    ssize_t res = read(fd, buffer, length);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _write(state *s) {
    fd_t fd = s->ctx->regs.ebx;
    void *buffer = (void *) s->ctx->regs.ecx;
    size_t length = (size_t) s->ctx->regs.esi;
    ssize_t res = write(fd, buffer, length);
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
    int res = mkdir(path, mode);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _rmdir(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    int res = rmdir(path);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _chdir(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    int res = chdir(path);
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
    fd_t res = opendir(path);
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
            if (s->processes[rq->hd].state.state == RUNNABLE) {//&& s->processes[rq->hd].slices_left > 0 ?
                next_pid = rq->hd;
                s->processes[next_pid].slices_left = MAX_TIME_SLICES;
                s->curr_pid = next_pid;
                s->curr_priority = p;
                user_pd = &global_state.processes[next_pid].page_directory;
                user_esp = global_state.processes[next_pid].saved_context.regs.esp - 0x2C;
                //kprintf("hi %d, %x\n", next_pid, s->processes[next_pid].saved_context.stack.eip);
                return;
            }
            rq = rq->tl;
        }

    }
    kprintf("No process to run...\n");
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
        /*
        syscall_t sc = decode(s);

        switch (sc.t) {
        case SEND:
            reorder_req = picosend(s, sc.ch_send, sc.val_send);
            break;

        case RECV:
            reorder_req = picoreceive(s, sc.ch_list);
            break;

        case FORK:
            //picofork(s, sc.priority, sc.v2, sc.v3, sc.v4);
            break;

        case WAIT:
            reorder_req = picowait(s);
            break;

        case EXIT:
            reorder_req = 1;
            picoexit(s);
            break;

        case NEWCHANNEL:
            piconew_channel(s);
            break;
        
        case GET_KEY_EVENT:
            picoget_key_event(s);
            break;
        
        
        case INVALID:
            break;
        }*/
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
        reorder(s);
    }
}

void focus_next_process() {
    do
        global_state.focus = (global_state.focus + 1) % NUM_PROCESSES;
    while(global_state.processes[global_state.focus].state.state == FREE);
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

#define CODE_LEN 0x10000
u8 TMP_BUFFER[CODE_LEN];
u8 *TMP_CODE = TMP_BUFFER;
page_directory_t *TMP_PD;

void start_process(int pid, int parent) {
    process *p = &global_state.processes[pid];
    p->parent_id = parent;
    p->state.state = RUNNABLE;
    p->slices_left = 0;
    p->state.ch_list = NULL;
    
    char *file = "/console.bin";
    
    page_directory_t *pd = init_user_page_dir(file, get_identity());
    if (pd == NULL) {
        kprintf("Not good : NULL page directory !\nErrno : %s", strerror(errno));
        asm("hlt");
    }
    kprintf("Starting process %d (code = %x) [%x]\n", pid, user_code, *(u32*)user_code);
    p->page_directory = pd;
    copy_context(global_state.ctx, &p->saved_context);
    p->saved_context.stack.eip = USER_CODE_VIRTUAL;
    p->saved_context.regs.esp = USER_STACK_VIRTUAL + 0x1000 - sizeof(context_t) - 0x8 + 0x2C;
    global_state.runqueues[MAX_PRIORITY] = add(pid, global_state.runqueues[MAX_PRIORITY]);
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
    //syscall_fun[3] = _fork;
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
    for (i = 0; i < NUM_CHANNELS; i++) {
        s->channels[i].recvs = NULL;
        s->channels[i].state = UNUSED;
        s->channels[i].s_pid = 0;
        s->channels[i].s_priority = 0;
        s->channels[i].s_value = 0;
    }

    for (i = 0; i < NUM_PROCESSES; i++) {
        s->processes[i].parent_id = 0;
        s->processes[i].state.state = FREE;
        s->processes[i].slices_left = 0;
        s->processes[i].state.ch_list = NULL;
    }

    for (i = 0; i <= MAX_PRIORITY; i++) {
        s->runqueues[i] = NULL;
    }
    for(int i = 0; i < 10; i++) start_process(i, i);
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
            kprintf("%d(%d)\n", q->hd, s->processes[q->hd].state.state);
            for (list *curr = q->tl; curr != NULL; curr = curr->tl) {
                kprintf("%d(%d)\n", curr->hd, s->processes[curr->hd].state.state);
            }
        }
    }

    kprintf("\nChannels:\n");
    for (chanid c = 0; c < NUM_CHANNELS; c++) {
        if (s->channels[c].state != UNUSED) {
            kprintf("%d: %d\n", c, s->channels[c].state);
        }
    }
    kprintf("\n");
}
