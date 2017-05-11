#include "lib.h"

int main() {
    printf("Hello!\n");
    /*
    if(!resize_heap(0x50000)) {
        printf("resize fail\n");
        exit(-1);
    }
    printf("PRE %x - %x\n", *((u32 *) 0x88008C04), *((u32 *) 0x88040BFB + 2));
    *((u32 *) 0x88008C04) = 0x1337;
    printf("PS* %x - %x\n", *((u32 *) 0x88008C04), *((u32 *) 0x88040BFB + 2));
    
    return 0;
    //*/
    init_malloc();
    u32 *x = (u32*) malloc(sizeof(u32));
    *x = 0xFEE;
    printf("x = %x, *x = %x\n", x, *x);
    
    u32 *y = (u32*) malloc(sizeof(u32));
    *y = 0x5;
    printf("y = %x, *y = %x\n", y, *y);
    
    free(y);
    printf("free y\n");
    
    u32 *z = (u32*) malloc(sizeof(u32));
    *z = 0xE;
    printf("z = %x, *z = %x\n", z, *z);
    
    u32 n = 0x80;
    u32 m = 0x1005;
    void ** pointers = (void **) malloc(n * sizeof(void **));
    for(int i = 0; i < n; i ++) pointers[i] = NULL;
    for(u32 ct = 0; ct < 0x1000; ct++) {
        print = ct >= 0x820;
        print = 0;
        if(print) {
            printf("# %x #\n", ct);
            malloc_header_t *b = (malloc_header_t*) 0x88040BFB;
            printf("BAD GUY: %x %x %x\n", b, b->prev_block, b->next_block);
        }
        int i = rand() % n;
        if(pointers[i]) free(pointers[i]);
        if(print) {
            malloc_header_t *b = (malloc_header_t*) 0x88040BFB;
            printf("BAD GUY: %x %x %x\n", b, b->prev_block, b->next_block);
        }
        pointers[i] = malloc(rand() % m);
        flush(STDOUT);
        if(!pointers[i]) break;
    }
}
