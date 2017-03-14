#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printing.h"

static const int CONSTANTE = 42;
//Variable que dans ce fichier la
//Dans une fonction, elle n'est definie qu'une fois
#define MAX_SIZE_LIST 100
#define MAX_SIZE_C_LIST 100


#define MAX_TIME_SLICES 5
#define MAX_PRIORITY 15
#define NUM_PROCESSES 32
#define NUM_CHANNELS 128
#define NUM_REGISTERS 5

typedef int pid;
typedef int chanid;
typedef int value;
typedef int interrupt;
typedef int priority;

typedef struct registers registers;
struct registers {
    int r0;
    int r1;
    int r2;
    int r3;
    int r4;
};

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

typedef enum p_state p_state;
enum p_state {
    FREE, BLOCKEDWRITING, BLOCKEDREADING, WAITING, RUNNABLE, ZOMBIE
};

typedef struct process_state process_state;
struct process_state {
    p_state state;
    list *ch_list;
    chanid ch;
};

typedef struct process process;
struct process {
    pid parent_id;
    process_state state;
    int slices_left;
    int saved_context[NUM_REGISTERS];
};

typedef enum c_state c_state;
enum c_state {
    UNUSED, SENDER, RECEIVER
};

typedef struct c_list c_list;
struct c_list {
    pid pid;
    priority priority;
    c_list *tl;
};

struct elt_c_list {
    int free;
    c_list elt;
};

struct elt_c_list c_list_memory[MAX_SIZE_C_LIST];

typedef struct channel_state channel_state;
struct channel_state {
    c_state state;
    pid s_pid;
    priority s_priority;
    value s_value;
    c_list *recvs;
};

typedef struct state state;
struct state {
    pid curr_pid;
    priority curr_priority;
    int registers[NUM_REGISTERS];
    process processes[NUM_PROCESSES];
    channel_state channels[NUM_CHANNELS];
    list* runqueues[MAX_PRIORITY+1];
};

state global_state;

list* malloc_list()
{
    static int base = 0;
    int i;
    
    for (i=base; i < MAX_SIZE_LIST; i++){
        if (list_memory[i].free == 0)
        {
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

void free_list(list* l)
{
    int i;
    for (i=0; i < MAX_SIZE_LIST; i++){
        if (&(list_memory[i].elt) == l)
        {
            list_memory[i].free = 0;
            return;
        }
    }
    kprintf("Invalid argument : no such list element to free.");
}

c_list* malloc_c_list()
{
    static int base = 0;
    int i;
    
    for (i=base; i < MAX_SIZE_C_LIST; i++){
        if (c_list_memory[i].free == 0)
        {
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

void free_c_list(c_list* l)
{
    int i;
    for (i=0; i < MAX_SIZE_C_LIST; i++){
        if (&(c_list_memory[i].elt) == l)
        {
            c_list_memory[i].free = 0;
            return;
        }
    }
    kprintf("Invalid argument : no such c_list element to free.");
}

list* add(int hd, list* tl)
{
    list *res = malloc_list();
    res->hd = hd;
    res->tl = tl;

    return res;
}

list *filter(list *l, int elt)
{
    //On ne le fait pas en place !
    if (l == NULL)
    {
        return NULL;
    }

    list *temp = filter(l->tl, elt);

    if (l->hd == elt)
    {
        free_list(l); // Ni de f
        return temp;
    }

    l->tl = temp;
    return l;
}


list *append(list *l, int elt)
{
    if (l == NULL)
    {
        list *res = malloc_list(sizeof(list));
        res->hd = elt;
        res->tl = NULL;
        return res;
    }

    l->tl = append(l->tl, elt);

    return l;
}

c_list* get_recv(state *s, chanid i)
{
    c_list* res = s->channels[i].recvs;
    s->channels[i].recvs = res->tl;
    return res;
}

c_list* remove_recv(pid r, c_list* l)
{
    //On ne le fait pas en place !
    if (l == NULL)
    {
        return NULL;
    }

    c_list *temp = remove_recv(r, l->tl);

    if (l->pid == r)
    {
        free_c_list(l);
        return temp;
    }

    l->tl = temp;
    return l;
}

void release_recv(state *s, pid r, list *ch_list)
{
    while (ch_list != NULL)
    {
        s->channels[ch_list->hd].recvs = remove_recv(r, s->channels[ch_list->hd].recvs);
        if (s->channels[ch_list->hd].recvs == NULL)
        {
            s->channels[ch_list->hd].state = UNUSED;
        }
        ch_list = ch_list->tl;
    }
}


c_list *add_recv(pid i, priority p, c_list* l)
{
    if (l == NULL || p > l->priority)
    {
        c_list *res = malloc_c_list();
        res->pid = i;
        res->priority = p;
        res->tl = l;

        return res;
    }

    l->tl = add_recv(i, p, l->tl);
    return l;
}


registers get_registers(int source[NUM_REGISTERS])
{
    registers res = { source[0], source[1],
                      source[2], source[3],
                      source[4] };
    return res;
}

void set_registers(int dest[NUM_REGISTERS], registers regs)
{
    dest[0] = regs.r0;
    dest[1] = regs.r1;
    dest[2] = regs.r2;
    dest[3] = regs.r3;
    dest[4] = regs.r4;
}


void set_registers_bis(int dest[NUM_REGISTERS], registers *regs)
{
    dest[0] = regs->r0;
    dest[1] = regs->r1;
    dest[2] = regs->r2;
    dest[3] = regs->r3;
    dest[4] = regs->r4;
}



pid get_current(state *s)
{
    return s->curr_pid;
}

typedef enum event event;
enum event { TIMER, SYSCALL };

typedef enum syscall_t syscall_t;
enum syscall_t { SEND, RECV, FORK, WAIT, EXIT, NEWCHANNEL, INVALID };

typedef struct syscall syscall;
struct syscall {
    syscall_t t;
    chanid ch_send;
    value val_send;
    list *ch_list;
    priority priority;
    value v2;
    value v3;
    value v4;
};

syscall decode(state *s)
{
    syscall res;

    switch (s->registers[0])
    {
    case 0:
        res.t = NEWCHANNEL;
        break;

    case 1:
        res.t = SEND;
        res.ch_send = s->registers[1];
        res.val_send = s->registers[2];
        break;

    case 2:
        res.t = RECV;
        res.ch_list = add(s->registers[1],
                          add(s->registers[2],
                              add(s->registers[3],
                                  add(s->registers[4],
                                      NULL))));
        break;

    case 3:
        res.t = FORK;
        res.priority = s->registers[1];
        res.v2 = s->registers[2];
        res.v3 = s->registers[3];
        res.v4 = s->registers[4];
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

void picofork(state *s, priority nprio, value v2, value v3, value v4)
{
    if (s->curr_priority < nprio)
    {
        s->registers[0] = 0;
        return;
    }

    //Good priority
    //Finds the new process
    int i;
    for (i = 0; i < NUM_PROCESSES; i++)
    {
        if (s->processes[i].state.state == FREE)
        {
            //Found a free process
            s->registers[0] = 1;
            s->registers[1] = i;

            //Creating the new process
            process *new_p = &(s->processes[i]);
            new_p->parent_id = s->curr_pid;
            new_p->slices_left = MAX_TIME_SLICES;
            int *context = new_p->saved_context; //VERIFIER
            context[0] = 2;
            context[1] = s->curr_pid;
            context[2] = v2;
            context[3] = v3;
            context[4] = v4;

            new_p->state.state = RUNNABLE;
            s->runqueues[nprio] = add(i, s->runqueues[nprio]);

            return;
        }
    }

    //No free process
    s->registers[0] = 0;
    return;
}

void picoexit(state *s)
{
    int i = s->curr_pid;
    s->processes[i].state.state = ZOMBIE;
    int pere = s->processes[i].parent_id;

    //On trouve les processus fils de celui ci.
    int j;
    for (j = 0; j < NUM_PROCESSES; j++)
    {
        if (s->processes[j].parent_id == i)
        {
            s->processes[j].parent_id = 1;
        }
    }

    //On cherche aussi le processus parent de celui-ci, s'il est en wait.
    if (s->processes[pere].state.state == WAITING)
    {
        //On fait la même chose que dans wait.
        s->processes[pere].state.state = RUNNABLE;
        s->processes[i].state.state = FREE;
        int *regs = s->processes[i].saved_context;
        regs[0] = 1;
        regs[1] = i;
        regs[2] = s->registers[0];
    }

    //On enlève le processus de sa file
    s->runqueues[s->curr_priority] = filter(s->runqueues[s->curr_priority], i);
}

int picowait(state *s)
{
    int i = s->curr_pid;
    int fils = 0;
    //On trouve un fils zombie
    int j;
    for (j = 0; j < NUM_PROCESSES; j++)
    {
        if (s->processes[j].parent_id == i)
        {
            fils = 1;
            if (s->processes[j].state.state == ZOMBIE)
            {
                s->registers[0] = 1;
                s->registers[1] = j;
                s->registers[2] = s->processes[j].saved_context[1];
                s->processes[j].state.state = FREE;
                return 0;
            }
        }
    }

    if (fils)
    {
        s->processes[i].state.state = WAITING;
        return 1;
    }

    s->registers[0] = 0;
    return 0;
}


void piconew_channel(state *s)
{
    int i;
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        if (s->channels[i].state == UNUSED)
        {
            //On a un channel de libre.
            //On le met a receivers vide
            s->channels[i].state = RECEIVER;
            s->channels[i].recvs = NULL;
            s->registers[0] = i;
            return;
        }
    }
    s->registers[0] = -1;
    return;
}


int picosend(state *s, chanid i, value v)
{
    pid writer = s->curr_pid;
    channel_state *ch = &(s->channels[i]);

    if (ch->state == UNUSED || ch->state == SENDER)
    {
        s->registers[0] = 0;
        return 0;
    }

    //Le canal est valide, on regarde s'il est vide.
    if (ch->recvs == NULL)
    {
        ch->state = SENDER;
        ch->s_value = v;
        ch->s_pid = s->curr_pid;
        ch->s_priority = s->curr_priority;
        s->processes[writer].state.state = BLOCKEDWRITING;
        s->processes[writer].state.ch = i;
        s->registers[0] = 1;
        return 1;
    }

    //Le canal n'est pas vide.
    c_list *r = get_recv(s, i);
    pid recv = r->pid;
    priority recv_p = r->priority;
    free_c_list(r);

    //On remet en etat le proccessus qui a lu la valeur.
    //On le retire de tous les channels ou il ecoute.
    list *ch_list = s->processes[recv].state.ch_list;
    release_recv(s, recv, ch_list);

    s->processes[recv].state.state = RUNNABLE;
    s->processes[recv].saved_context[0] = 1;
    s->processes[recv].saved_context[1] = i;
    s->processes[recv].saved_context[2] = v;
    s->registers[0] = 1;

    //On ne sait pas trop ce que renvoie cette fonction
    return recv_p > s->curr_priority;
}


int picoreceive(state *s, list *ch_list)
{
    int valide = 0;

    list *next_ch = ch_list;
    while (next_ch != NULL)
    {
        chanid ch_id = next_ch->hd;

        if (ch_id >= 0 && ch_id < NUM_CHANNELS)
        {
            channel_state *ch = &(s->channels[ch_id]);
            if (ch->state == SENDER)
            {
                s->registers[0] = 1;
                s->registers[1] = ch_id;
                s->registers[2] = ch->s_value;
                s->processes[ch->s_pid].state.state = RUNNABLE;


                ch->state = UNUSED;
                ch->recvs = NULL;
                release_recv(s, s->curr_pid, ch_list);
                return ch->s_priority >= s->curr_priority;
            }
            else if (ch->state == RECEIVER)
            {
                valide = 1;
                ch->recvs = add_recv(s->curr_pid, s->curr_priority, ch->recvs);
            }
        }
        next_ch = next_ch->tl;

    }

    if (valide)
    {
        s->processes[s->curr_pid].state.state = BLOCKEDREADING;
        s->processes[s->curr_pid].state.ch_list = ch_list;
        //s->registers[0] = 0;
        return 1;
    }

    s->registers[0] = 0;
    return 0;
}


void picotransition(state *s, event ev)
{
    int reorder = 0;
    if (ev == SYSCALL)
    {
        syscall sc = decode(s);

        switch (sc.t)
        {
        case SEND:
            reorder = picosend(s, sc.ch_send, sc.val_send);
            break;

        case RECV:
            reorder = picoreceive(s, sc.ch_list);
            break;

        case FORK:
            picofork(s, sc.priority, sc.v2, sc.v3, sc.v4);
            break;

        case WAIT:
            reorder = picowait(s);
            break;

        case EXIT:
            reorder = 1;
            picoexit(s);
            break;

        case NEWCHANNEL:
            piconew_channel(s);
            break;

        case INVALID:
            break;
        }
    }
    else
    {
        s->processes[s->curr_pid].slices_left --;
        if (s->processes[s->curr_pid].slices_left == 0)
        {
            reorder = 1;
            //On remet le processus a la fin
            priority p = s->curr_priority;
            pid id = s->curr_pid;
            s->runqueues[p] = append(filter(s->runqueues[p], id), id);
        }
    }

    pid next_pid;
    if (reorder)
    {
        //On reelit un processus
        priority p;
        list *rq;
        for (p = MAX_PRIORITY; p >= 0; p--)
        {
            rq = s->runqueues[p];
            while (rq != NULL)
            {
                if (s->processes[rq->hd].state.state == RUNNABLE) //&& s->processes[rq->hd].slices_left > 0 ?
                {
                    next_pid = rq->hd;
                    //s->processes[next_pid].slices_left = MAX_TIME_SLICES;
                    //On sauvegarde les registres
                    set_registers(s->processes[s->curr_pid].saved_context, get_registers(s->registers));
                    set_registers(s->registers, get_registers(s->processes[next_pid].saved_context));
                    s->curr_pid = next_pid;
                    s->curr_priority = p;

                    return;
                }
                rq = rq->tl;
            }

        }
        kprintf("Dommage... \n");
        //fprintf(stderr, "No process to run, undefined behavior.\n");
    }
}

state *picoinit()
{
    state *s = &global_state;
    s->curr_pid = 1;
    s->curr_priority = MAX_PRIORITY;

    int i;
    for (i = 0; i < NUM_CHANNELS; i++)
    {
        s->channels[i].recvs = NULL;
        s->channels[i].state = UNUSED;
        s->channels[i].s_pid = 0;
        s->channels[i].s_priority = 0;
        s->channels[i].s_value = 0;
    }

    for (i = 0; i < NUM_PROCESSES; i++)
    {
        if (i == 0)
        {
            s->processes[i].parent_id = 0;
            s->processes[i].state.state = RUNNABLE;
            s->processes[i].slices_left = MAX_TIME_SLICES;
            s->processes[i].state.ch_list = NULL;
        }
        else if (i == 1)
        {
            s->processes[i].parent_id = 1;
            s->processes[i].state.state = RUNNABLE;
            s->processes[i].slices_left = MAX_TIME_SLICES;
            s->processes[i].state.ch_list = NULL;
        }
        else
        {
            s->processes[i].parent_id = 0;
            s->processes[i].state.state = FREE;
            s->processes[i].slices_left = 0;
            s->processes[i].state.ch_list = NULL;
        }

        int j;
        for (j = 0; j < NUM_REGISTERS; j++)
        {
            s->processes[i].saved_context[j] = 0;
        }
    }

    for (i = 0; i <= MAX_PRIORITY; i++)
    {
        if (i == MAX_PRIORITY)
        {
            s->runqueues[i] = add(1, NULL);
        }
        else if (i == 0)
        {
            s->runqueues[i] = add(0, NULL);
        }
        else
        {
            s->runqueues[i] = NULL;
        }
    }

    set_registers(s->registers, get_registers(s->processes[0].saved_context));

    return s;
}

/*
char* process_to_str(process p)
{
    char* state = malloc(100 * sizeof(char));
    switch (p.state.state)
    {
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

char* channel_to_str(channel_state c)
{
    char* state = malloc(100 * sizeof(char));
    switch (c.state)
    {
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

void log_state(state* s)
{
    kprintf("Current process is %d (priority %d, %d slices left)\n",
        s->curr_pid, s->curr_priority, s->processes[s->curr_pid].slices_left);
    kprintf("Registers are: r0=%d, r1=%d, r2=%d, r3=%d, r4=%d\n",
        s->registers[0], s->registers[1], s->registers[2], s->registers[3], s->registers[4]);

    kprintf("\nRunqueues:\n");
    for (priority prio = MAX_PRIORITY; prio >= 0; prio--)
    {
        list* q = s->runqueues[prio];
        if (q != NULL)
        {
            kprintf("Priority %d:\n", prio);
            kprintf("%d(%d)\n", q->hd, s->processes[q->hd].state.state);
            for (list *curr = q->tl; curr != NULL; curr = curr->tl)
            {
                kprintf("%d(%d)\n", curr->hd, s->processes[curr->hd].state.state);
            }
        }
    }

    kprintf("\nChannels:\n");
    for (chanid c = 0; c < NUM_CHANNELS; c++)
    {
        if (s->channels[c].state != UNUSED)
        {
            kprintf("%d: %d\n", c, s->channels[c].state);
        }
    }
    kprintf("\n\n");
}


int launch()
{
    kprintf("Initial state\n");
    state* s = picoinit();
    log_state(s);

    kprintf("Forking init\n");
    s->registers[0] = 3;
    s->registers[1] = MAX_PRIORITY;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Asking for a new channel, in r4\n");
    s->registers[0] = 0;
    picotransition(s, SYSCALL);
    s->registers[4] = s->registers[0];
    log_state(s);

    kprintf("Making init wait for a message on the channel\n");
    kprintf("This should switch to the child process since init is BlockedReading\n");
    s->registers[0] = 2;
    s->registers[1] = -1;
    s->registers[2] = -1;
    s->registers[3] = -1;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Getting a new channel in r3\n");
    s->registers[0] = 0;
    picotransition(s, SYSCALL);
    s->registers[3] = s->registers[0];
    log_state(s);

    kprintf("What about having a child of our own?\n");
    s->registers[0] = 3;
    s->registers[1] = MAX_PRIORITY - 1;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Let's wait for him to die!\n");
    s->registers[0] = 5;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("On with the grandchild, which'll send on channel r3\n");
    s->registers[0] = 1;
    s->registers[1] = s->registers[3];
    s->registers[2] = -12;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("On with idle, to listen to the grandchild!\n");
    s->registers[0] = 2;
    s->registers[1] = 1; // Little hack, not supposed to know it's gonna be channel one
    s->registers[2] = -1;
    s->registers[3] = -1;
    s->registers[4] = -1;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Letting the timer tick until we're back to the grandchild\n");
    for (int i = MAX_TIME_SLICES; i >= 0; i--)
    {
        picotransition(s, TIMER);
    }
    log_state(s);

    kprintf("Hara-kiri\n");
    s->registers[0] = 4;
    s->registers[1] = 125;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Let's speak to dad!\n");
    s->registers[0] = 1;
    s->registers[1] = s->registers[4];
    s->registers[2] = 42;
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Our job is done, back to dad! (see 42 in r2?)\n");
    s->registers[0] = 4;
    s->registers[1] = 12; // Return value
    picotransition(s, SYSCALL);
    log_state(s);

    kprintf("Let's loot the body of our child (see 12 in r2?)\n");
    s->registers[0] = 5;
    picotransition(s, SYSCALL);
    log_state(s);
    
    return 0;
}

