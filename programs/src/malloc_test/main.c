#include "lib.h"

int main() {
    printf("Hello!\n");
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
    
    void **pointers = (void **) malloc(0x6 * sizeof(void **));
    
    for(int i = 0; i < 0x6; i ++) pointers[i] = malloc(0x10);
    free(pointers[1]);
    pointers[1] = malloc(0x10);
    
    
    /*void ** pointers = (void **) malloc(0x8 * sizeof(void **));
    for(int i = 0; i < 0x20; i ++) pointers[i] = NULL;*/
    for(;;) {
        /*void *p = pointers[rand() % 0x8];
        if(p) free(p);
        p = malloc(rand() % 0x10);*/
        flush(STDOUT);
    }
}
