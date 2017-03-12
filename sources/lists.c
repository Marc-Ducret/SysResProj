#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel.c"

#define MAX_SIZE_LIST 100


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

list* malloc_list()
{
    static int base = 0;
    int i;
    
    for (i=base; i < MAX_SIZE_LIST; i++){
        if (list_memory[i].free == 0)
        {
            base = i + 1;
            if (base = MAX_SIZE_LIST)
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

