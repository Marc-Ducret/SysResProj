#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printing.h"
#include "kernel.h"
#include "lib.h"

static const int CONSTANTE = 42;
//Variable que dans ce fichier la
//Dans une fonction, elle n'est definie qu'une fois

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

    case 3:
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

void picoexit(state *s) {
    int i = s->curr_pid;
    s->processes[i].state.state = ZOMBIE;
    int pere = s->processes[i].parent_id;

    //On trouve les processus fils de celui ci.
    int j;
    for (j = 0; j < NUM_PROCESSES; j++) {
        if (s->processes[j].parent_id == i) {
            s->processes[j].parent_id = 1;
        }
    }

    //On cherche aussi le processus parent de celui-ci, s'il est en wait.
    if (s->processes[pere].state.state == WAITING) {
        //On fait la même chose que dans wait.
        s->processes[pere].state.state = RUNNABLE;
        s->processes[i].state.state = FREE;
        registers_t *regs = &(s->processes[i].saved_context.regs);
        regs->eax = 1;
        regs->ebx = i;
        regs->ecx = s->ctx->regs.eax;
    }

    //On enlève le processus de sa file
    s->runqueues[s->curr_priority] = filter(s->runqueues[s->curr_priority], i);
}

int picowait(state *s) {
    int i = s->curr_pid;
    int fils = 0;
    //On trouve un fils zombie
    int j;
    for (j = 0; j < NUM_PROCESSES; j++) {
        if (s->processes[j].parent_id == i) {
            fils = 1;
            if (s->processes[j].state.state == ZOMBIE) {
                s->ctx->regs.eax = 1;
                s->ctx->regs.ebx = j;
                s->ctx->regs.ecx = s->processes[j].saved_context.regs.ebx;
                s->processes[j].state.state = FREE;
                return 0;
            }
        }
    }

    if (fils) {
        s->processes[i].state.state = WAITING;
        return 1;
    }

    s->ctx->regs.eax = 0;
    return 0;
}


void piconew_channel(state *s) {
    int i;
    for (i = 0; i < NUM_CHANNELS; i++) {
        if (s->channels[i].state == UNUSED) {
            //On a un channel de libre.
            //On le met a receivers vide
            s->channels[i].state = RECEIVER;
            s->channels[i].recvs = NULL;
            s->ctx->regs.eax = i;
            return;
        }
    }
    s->ctx->regs.eax = -1;
    return;
}


int picosend(state *s, chanid i, value v) {
    pid_t writer = s->curr_pid;
    channel_state *ch = &(s->channels[i]);

    if (ch->state == UNUSED || ch->state == SENDER) {
        s->ctx->regs.eax = 0;
        return 0;
    }

    //Le canal est valide, on regarde s'il est vide.
    if (ch->recvs == NULL) {
        ch->state = SENDER;
        ch->s_value = v;
        ch->s_pid = s->curr_pid;
        ch->s_priority = s->curr_priority;
        s->processes[writer].state.state = BLOCKEDWRITING;
        s->processes[writer].state.ch = i;
        s->ctx->regs.eax = 1;
        return 1;
    }

    //Le canal n'est pas vide.
    c_list *r = get_recv(s, i);
    pid_t recv = r->pid;
    priority recv_p = r->priority;
    free_c_list(r);

    //On remet en etat le proccessus qui a lu la valeur.
    //On le retire de tous les channels ou il ecoute.
    list *ch_list = s->processes[recv].state.ch_list;
    release_recv(s, recv, ch_list);

    s->processes[recv].state.state = RUNNABLE;
    s->processes[recv].saved_context.regs.eax = 1;
    s->processes[recv].saved_context.regs.ebx = i;
    s->processes[recv].saved_context.regs.ecx = v;
    s->ctx->regs.eax = 1;

    //On ne sait pas trop ce que renvoie cette fonction
    return recv_p > s->curr_priority;
}


int picoreceive(state *s, list *ch_list) {
    int valide = 0;

    list *next_ch = ch_list;
    while (next_ch != NULL) {
        chanid ch_id = next_ch->hd;

        if (ch_id >= 0 && ch_id < NUM_CHANNELS) {
            channel_state *ch = &(s->channels[ch_id]);
            if (ch->state == SENDER) {
                s->ctx->regs.eax = 1;
                s->ctx->regs.ebx = ch_id;
                s->ctx->regs.ecx = ch->s_value;
                s->processes[ch->s_pid].state.state = RUNNABLE;


                ch->state = UNUSED;
                ch->recvs = NULL;
                release_recv(s, s->curr_pid, ch_list);
                return ch->s_priority >= s->curr_priority;
            } else if (ch->state == RECEIVER) {
                valide = 1;
                ch->recvs = add_recv(s->curr_pid, s->curr_priority, ch->recvs);
            }
        }
        next_ch = next_ch->tl;

    }

    if (valide) {
        s->processes[s->curr_pid].state.state = BLOCKEDREADING;
        s->processes[s->curr_pid].state.ch_list = ch_list;
        //s->ctx->regs.eax = 0;
        return 1;
    }

    s->ctx->regs.eax = 0;
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
                copy_context(&(s->processes[next_pid].saved_context), s->ctx);
                s->curr_pid = next_pid;
                s->curr_priority = p;

                return;
            }
            rq = rq->tl;
        }

    }
    kprintf("No process to run...\n");
    return;
}

void picotransition(state *s, event ev) {
    copy_context(s->ctx, &(s->processes[s->curr_pid].saved_context));
    int reorder_req = 0;
    if (ev == SYSCALL) {
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

        case INVALID:
            break;
        }
    } else {
        s->processes[s->curr_pid].slices_left --;
        if (s->processes[s->curr_pid].slices_left == 0) {
            reorder_req = 1;
            //On remet le processus a la fin
            priority p = s->curr_priority;
            pid_t id = s->curr_pid;
            s->runqueues[p] = add(id, filter(s->runqueues[p], id));
        }
    }

    if (reorder_req) {
        reorder(s);
    }
}

void picosyscall(context_t *ctx) {
    // Calls the picotransition with current registers pointing regs.
    global_state.ctx = ctx;
    picotransition(&global_state, SYSCALL);
}

void picotimer(context_t *ctx) {
    // Calls the picotransition with current registers pointing regs.
    global_state.ctx = ctx;
    picotransition(&global_state, TIMER);
}

state *picoinit(context_t *ctx) {
    state *s = &global_state;
    s->curr_pid = 1;
    s->curr_priority = MAX_PRIORITY;
    s->ctx = ctx;
    
    int i;
    for (i = 0; i < NUM_CHANNELS; i++) {
        s->channels[i].recvs = NULL;
        s->channels[i].state = UNUSED;
        s->channels[i].s_pid = 0;
        s->channels[i].s_priority = 0;
        s->channels[i].s_value = 0;
    }

    for (i = 0; i < NUM_PROCESSES; i++) {
        if (i == 0) {
            s->processes[i].parent_id = 0;
            s->processes[i].state.state = RUNNABLE;
            s->processes[i].slices_left = MAX_TIME_SLICES;
            s->processes[i].state.ch_list = NULL;
        } else if (i == 1) {
            s->processes[i].parent_id = 1;
            s->processes[i].state.state = RUNNABLE;
            s->processes[i].slices_left = MAX_TIME_SLICES;
            s->processes[i].state.ch_list = NULL;
        } else {
            s->processes[i].parent_id = 0;
            s->processes[i].state.state = FREE;
            s->processes[i].slices_left = 0;
            s->processes[i].state.ch_list = NULL;
        }

        int j;
        for (j = 0; j < NUM_REGISTERS; j++) {
            //init_registers(&(s->processes[i].saved_context)); TODO useless?? fct add process...
        }
    }

    for (i = 0; i <= MAX_PRIORITY; i++) {
        if (i == MAX_PRIORITY) {
            s->runqueues[i] = add(1, NULL);
        } else if (i == 0) {
            s->runqueues[i] = add(0, NULL);
        } else {
            s->runqueues[i] = NULL;
        }
    }

    //init_registers(s->ctx->regs);
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


void launch() {
    kprintf("Initial state\n");
    context_t ctx;
    state* s = picoinit(&ctx);
    log_state(s);

    kprintf("Forking init\n");
    s->ctx->regs.eax = 3;
    s->ctx->regs.ebx = MAX_PRIORITY;
    picotransition(s, SYSCALL);
    log_state(s);
    
    kprintf("Asking for a new channel, in r4\n");
    s->ctx->regs.eax = 0;
    picotransition(s, SYSCALL);
    s->ctx->regs.esi = s->ctx->regs.eax;
    log_state(s);

    kprintf("Making init wait for a message on the channel\n");
    kprintf("This should switch to the child process since init is BlockedReading\n");
    s->ctx->regs.eax = 2;
    s->ctx->regs.ebx = -1;
    s->ctx->regs.ecx = -1;
    s->ctx->regs.edx = -1;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Getting a new channel in r3\n");
    s->ctx->regs.eax = 0;
    picotransition(s, SYSCALL);
    s->ctx->regs.edx = s->ctx->regs.eax;
    log_state(s);

    kprintf("What about having a child of our own?\n");
    s->ctx->regs.eax = 3;
    s->ctx->regs.ebx = MAX_PRIORITY - 1;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Let's wait for him to die!\n");
    s->ctx->regs.eax = 5;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("On with the grandchild, which'll send on channel r3\n");
    s->ctx->regs.eax = 1;
    s->ctx->regs.ebx = s->ctx->regs.edx;
    s->ctx->regs.ecx = -12;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("On with idle, to listen to the grandchild!\n");
    s->ctx->regs.eax = 2;
    s->ctx->regs.ebx = 1; // Little hack, not supposed to know it's gonna be channel one
    s->ctx->regs.ecx = -1;
    s->ctx->regs.edx = -1;
    s->ctx->regs.esi = -1;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Letting the timer tick until we're back to the grandchild\n");
    for (int i = MAX_TIME_SLICES; i >= 0; i--) {
        picotransition(s, TIMER);
    }
    log_state(s);

    kprintf("Hara-kiri\n");
    s->ctx->regs.eax = 4;
    s->ctx->regs.ebx = 125;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Let's speak to dad!\n");
    s->ctx->regs.eax = 1;
    s->ctx->regs.ebx = s->ctx->regs.esi;
    s->ctx->regs.ecx = 42;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Our job is done, back to dad! (see 42 in r2?)\n");
    s->ctx->regs.eax = 4;
    s->ctx->regs.ebx = 12; // Return value
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Let's loot the body of our child (see 12 in r2?)\n");
    s->ctx->regs.eax = 5;
    picotransition(s, SYSCALL);
    log_state(s);
}
