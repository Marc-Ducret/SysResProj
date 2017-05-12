#include "kernel.h"

syscall_fun_t syscall_fun[NUM_SYSCALLS];
volatile int no_process = 0;
int hanging_pd = 0;

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

int start_process(int parent, char* file, char *args, int chin, int chout) {    
    fd_t fd = fopen(file, O_RDONLY);
    if (fd == -1)
        return -1;
    
    pid_t pid = 0;
    while(global_state.processes[pid].state != FREE) {
        if(++pid == NUM_PROCESSES) {
            close(fd);
            errno = EMPROC;
            return -1;
        }
    }
    
    page_directory_t *pd = init_user_page_dir(fd, args, get_identity());
    if (pd == NULL) {
        int err = errno;
        close(fd);
        errno = err;
        return -1;
    }
    
    process *p = &global_state.processes[pid];
    p->parent_id = parent;
    p->state = RUNNABLE;
    p->slices_left = 0;
    
    for (int j = 0; j < NUM_CHANNELS_PROC; j++) {
        p->channels[j].chanid = -1;
    }
    
    p->channels[0].chanid = chin;
    p->channels[0].write  = 0;
    p->channels[0].read   = 1;
    p->channels[1].chanid = chout;
    p->channels[1].write  = 1;
    p->channels[1].read   = 0;
    p->cwd = opendir(CUR_DIR_NAME);
    if (strlen(file) >= 255)
        file[255] = 0;
    strCopy(file, p->name);
    p->page_directory = pd;
    copy_context(global_state.ctx, &p->saved_context);
    p->saved_context.stack.eip = USER_CODE_VIRTUAL;
    p->saved_context.regs.esp = USER_STACK_VIRTUAL + 0x1000 - sizeof(context_t) - 0x8 + 0x2C;
    p->heap_pointer = (void *) USER_HEAP;
    global_state.runqueues[MAX_PRIORITY] = add(pid, global_state.runqueues[MAX_PRIORITY]);
    return pid;
}

void kill_process(pid_t pid) {
    state *s = &global_state;
    s->processes[pid].state = ZOMBIE;
    pid_t pere = s->processes[pid].parent_id;

    hanging_pd = 1;
    //On trouve les processus fils de celui ci.
    pid_t j;
    for (j = 0; j < NUM_PROCESSES; j++) {
        if (s->processes[j].parent_id == pid) {
            s->processes[j].parent_id = INIT_PROCESS; // TODO No hardcode of init process
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
    s->runqueues[MAX_PRIORITY] = filter(s->runqueues[MAX_PRIORITY], pid); //TODO priority?
    
    close(s->processes[pid].cwd);
}

int _exit(state *s) {
    kill_process(s->curr_pid);    
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
    ssize_t res = wait_channel(s);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    // Need to reorder only if there was no error and nothing left.
    return (res == 0);
}

int _receive(state *s) {
    ssize_t len = receive(s);
    s->ctx->regs.eax = len;
    s->ctx->regs.ebx = errno;
    return 0; // No need to reorder
}

int _sleep(state *s) {
    int time = s->ctx->regs.ebx;
    int res;
    if (time > 0) {
        pid_t pid = s->curr_pid;
        s->processes[pid].state = SLEEPING;
        c_list *curr = NULL;
        c_list *next = s->sleeping;
        c_list *new = malloc_c_list();
        int cur_time = 0;
        int next_time = 0;
        while (next != NULL) {
            next_time = cur_time + next->priority;
            if (next_time > time) {
                next->priority -= time - cur_time;
                break;
            }
            cur_time = next_time;
            curr = next;
            next = curr->tl;
        }
        if (curr != NULL)
            curr->tl = new;
        new->tl = next;
        new->pid = pid;
        new->priority = time - cur_time;
        if (curr == NULL)
            s->sleeping = new;
        res = 0;
    }
    else {
        errno = EINVAL;
        res = -1;
    }
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return (res == 0);
}

int _pinfo(state *s) {
    pid_t pid = s->ctx->regs.ebx;
    process_info_t *data = (process_info_t *) s->ctx->regs.ecx;
    
    int res = -1;
    if (check_address(data, 1, 1, s->processes[s->curr_pid].page_directory) == 0)
        res = get_pinfo(pid, data);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
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

int _remove(state *s) {
    char *path = (char *) s->ctx->regs.ebx;
    int res = -1;
    if (check_address(path, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = remove(path);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _fcopy(state *s) {
    char *src = (char *) s->ctx->regs.ebx;
    char *dest = (char *) s->ctx->regs.ecx;
    int res = -1;
    if (check_address(dest, 1, 0, s->processes[s->curr_pid].page_directory) == 0
        && check_address(src, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = copyfile(src, dest);
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
    s->processes[s->curr_pid].cwd = cwd;
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _getcwd(state *s) {
    char *buffer = (char *) s->ctx->regs.ebx;
    int res = -1;
    if (check_address(buffer, 1, 1, s->processes[s->curr_pid].page_directory) == 0) {
        char *kbuffer = getcwd();
        if (kbuffer != NULL) {
            strCopy(kbuffer, buffer);
            res = 0;
        }
    }
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
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
    user_dirent_t *user_dirent = (user_dirent_t*) s->ctx->regs.ecx;
    int res = -1;
    if (check_address(user_dirent, 1, 1, s->processes[s->curr_pid].page_directory) == 0) {
        dirent_t *dirent = readdir(fd);
        if (dirent != NULL) {
            user_dirent->mode = dirent->mode;
            strCopy(dirent->name, user_dirent->name);
            user_dirent->size = dirent->size;
            user_dirent->type = dirent->type;
            res = 0;
        }
    }
    s->ctx->regs.eax = res;
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

int _gettimeofday(state *s) {
    rtc_time_t *t = (rtc_time_t*) s->ctx->regs.ebx;
    int res = -1;
    if (check_address(t, 1, 1, s->processes[s->curr_pid].page_directory) == 0) {
        update_time();
        *t = current_time;
    }
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _exec(state *s) {
    char *file = (char *) s->ctx->regs.ebx;
    char *args = (char *) s->ctx->regs.ecx;
    int check2;
    if (!args) {
        args = "";
        check2 = 1;
    }
    else
        check2 = check_address(args, 1, 0, s->processes[s->curr_pid].page_directory) == 0;
    int chin  = (int)s->ctx->regs.esi < 0 ? -1 : s->processes[s->curr_pid].channels[s->ctx->regs.esi].chanid;
    int chout = (int)s->ctx->regs.edi < 0 ? -1 : s->processes[s->curr_pid].channels[s->ctx->regs.edi].chanid;
    // TODO check validity of channels ! (rights and existence)
    pid_t res = -1;
    if (check2 && 
        check_address(file, 1, 0, s->processes[s->curr_pid].page_directory) == 0)
        res = start_process(s->curr_pid, file, args, chin, chout);
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return 0;
}

int _kill(state *s) {
    pid_t pid = s->ctx->regs.ebx;
    int res;
    if (pid < 0 || pid >= NUM_PROCESSES || s->processes[pid].state == FREE || s->processes[pid].state == ZOMBIE) {
        res = -1;
        errno = EINVAL;
    }
    else {
        s->processes[pid].saved_context.regs.ebx = EXIT_KILL;
        kill_process(pid);
        res = 0;
    }
    s->ctx->regs.eax = res;
    s->ctx->regs.ebx = errno;
    return (pid == s->curr_pid);
}

int _resize_heap(state *s) {
    int delta_size = (int) s->ctx->regs.ebx;
    process *p = &s->processes[s->curr_pid];
    if(p->heap_pointer + delta_size < (void *) USER_HEAP || 
      (delta_size >= 0 && (u32) p->heap_pointer >= 0xFFFFFFFF - delta_size)) {
        s->ctx->regs.eax = 0;
        s->ctx->regs.ebx = 0xFFFFFFFF - delta_size; //TODO real error
        return 0;
    }
    u32 cur_page = (u32)(p->heap_pointer            -1) >> 12;
    u32 new_page = (u32)(p->heap_pointer+delta_size -1) >> 12;
    for(u32 i = new_page+1; i <= cur_page; i++) {
        free_page(get_page(i << 12, 0, p->page_directory), i << 12);
    }
    for(u32 i = cur_page+1; i <= new_page; i++) {
        page_t *page = get_page(i << 12, 1, p->page_directory);
        if(!page->present)
            map_page(page, 0, 0, 1);
    }
    s->ctx->regs.eax = (u32) p->heap_pointer;
    p->heap_pointer += delta_size;
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
    while (1) {
        for (p = MAX_PRIORITY; p >= 0; p--) {
            rq = s->runqueues[p];
            while (rq != NULL) {
                if (s->processes[rq->hd].state == RUNNABLE) {
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
        //kprintf("No process to run...\n");
        //log_state(&global_state);
        //asm("hlt");
        // TODO check this would be correct :
        no_process = 1;
        asm("sti");
        for (int i =0; i < 10; i++)
            asm("hlt");
        asm("cli");
        no_process = 0;
    }
}

void update_cursor(u8 x, u8 y) {
    u16 pos = (x == 0xFF && y == 0xFF) ? 0xFFFF : x + VGA_WIDTH * y;
    outportb(0x3D4, 0x0F);
    outportb(0x3D5, (u8) (pos & 0xFF));
    outportb(0x3D4, 0x0E);
    outportb(0x3D5, (u8) ((pos >> 8) & 0xFF));
}

void picotransition(state *s, event ev) {
    if(s->curr_pid >= 0) {
        copy_context(s->ctx, &(s->processes[s->curr_pid].saved_context));
        cwd = s->processes[s->curr_pid].cwd;
    }
    int reorder_req = 0;
    int time_slices = 0;
    if (ev == SYSCALL) {
        time_slices = SYSCALL_SLICES;
        int id = s->ctx->regs.eax;
        if (id >= 0 && id < NUM_SYSCALLS) {
            reorder_req = (syscall_fun[id])(s);
        }
        else {
            s->ctx->regs.eax = -1;
            s->ctx->regs.ebx = ENOSYS;
        }
    } else {
        time_slices = TIMER_SLICES;
    }
    // Updates current process time slices.
    if(s->curr_pid >= 0) {
        s->processes[s->curr_pid].slices_left -= time_slices;
        if (s->processes[s->curr_pid].slices_left <= 0) {
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
    
    if (reorder_req) {
        if(global_state.curr_pid == global_state.focus) {
            if(*((u16*) USER_SCREEN_VIRTUAL)) {
            memcpy((void*) 0xB8000, (void*) USER_SCREEN_VIRTUAL, SCREEN_SIZE);
            u8 x = *((u8*)USER_SCREEN_VIRTUAL + SCREEN_SIZE);
            u8 y = *((u8*)USER_SCREEN_VIRTUAL + SCREEN_SIZE + 1);
            update_cursor(x, y);
            } else
                focus_next_process();
        }
        copy_context(s->ctx, &(s->processes[s->curr_pid].saved_context)); // TODO remove redundant saves ?
        reorder(s);
    }
}

void focus_next_process() {
    do
        global_state.focus = (global_state.focus + 1) % NUM_PROCESSES;
    while(global_state.processes[global_state.focus].state == FREE);
    if(global_state.processes[global_state.focus].state != RUNNABLE) {
        //kprintf("You are looking at a blocked processus. Thus, it isn't able to display its screen, because it isn't updated !\n");
        //log_state(&global_state); TODO do elsewhere
    }
    while(nextKeyEvent() >= 0);
}

void picosyscall(context_t *ctx) {
    // Calls the picotransition with current registers pointing regs.
    global_state.ctx = ctx;
    picotransition(&global_state, SYSCALL);
}

u32 did_init = 0;

void picotimer(context_t *ctx) {
    if(hanging_pd) {
        hanging_pd = 0;
        for(int i = 0; i < NUM_PROCESSES; i ++) {
            if(global_state.processes[i].state == FREE && global_state.processes[i].page_directory) {
                if(i != global_state.curr_pid) {
                    free_page_directory(global_state.processes[i].page_directory);
                    global_state.processes[i].page_directory = NULL;
                } else hanging_pd = 1;
            }
        }
    }  
    // Update sleeping processes
    state *s = &global_state;
    //if (no_process)
    //    kprintf("Sleepig : %x\n", s->sleeping);
    if (s->sleeping != NULL) {
        s->sleeping->priority -= 1;
        if (s->sleeping->priority <= 0) {
            s->processes[s->sleeping->pid].state = RUNNABLE;
            c_list *next = s->sleeping->tl;
            free_c_list(s->sleeping);
            s->sleeping = next;
        }
    }
    if (no_process)
        return;
    
    // Calls the picotransition with current registers pointing regs.
    global_state.ctx = ctx;
    if(!did_init) {
        did_init = 1;
        picoinit();
        return;
    }
    if(global_state.curr_pid == global_state.focus) {
        if(*((u16*) USER_SCREEN_VIRTUAL)) {
            memcpy((void*) 0xB8000, (void*) USER_SCREEN_VIRTUAL, SCREEN_SIZE);
            u8 x = *((u8*)USER_SCREEN_VIRTUAL + SCREEN_SIZE);
            u8 y = *((u8*)USER_SCREEN_VIRTUAL + SCREEN_SIZE + 1);
            update_cursor(x, y);
        } else
            focus_next_process();
    }
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
    syscall_fun[8] = _sleep;
    syscall_fun[9] = _pinfo;
    
    syscall_fun[10] = _fopen;
    syscall_fun[11] = _close;
    syscall_fun[12] = _read;
    syscall_fun[13] = _write;
    syscall_fun[14] = _seek;
    syscall_fun[15] = _remove;
    syscall_fun[16] = _fcopy;
    
    syscall_fun[20] = _mkdir;
    syscall_fun[21] = _rmdir;
    syscall_fun[22] = _chdir;
    syscall_fun[23] = _getcwd;
    syscall_fun[24] = _opendir;
    syscall_fun[25] = _readdir;
    syscall_fun[26] = _rewinddir;
    syscall_fun[27] = _closedir;
    
    syscall_fun[40] = _get_key_event;
    syscall_fun[41] = _gettimeofday;
    syscall_fun[42] = _kill;
    syscall_fun[43] = _resize_heap;
}

state *picoinit() {
    init_syscalls_table();
    state *s = &global_state;
    s->curr_pid = -1;
    s->curr_priority = MAX_PRIORITY;
    s->sleeping = NULL;
    
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
    start_process(0, "/console.bin", "/shell.bin", -1, -1);
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
int get_pinfo(pid_t pid, process_info_t *data) {
    state *s = &global_state;
    if (pid < 0 || pid >= NUM_PROCESSES) {
        errno = EINVAL;
        return -1;
    }
    strCopy(s->processes[pid].name, data->name);
    data->parent = s->processes[pid].parent_id;
    data->pid = pid;
    data->state = s->processes[pid].state;
    return 0;
}

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
