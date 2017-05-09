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
    
    u8 run = 1;
    
    u32 n = 0x80;
    u32 m = 0x1000;
    void ** pointers = (void **) malloc(n * sizeof(void **));
    for(int i = 0; i < n; i ++) pointers[i] = NULL;
    for(;;) {
        if(run) {
            int i = rand() % n;
            if(pointers[i]) free(pointers[i]);
            pointers[i] = malloc(rand() % m);
            if(!pointers[i]) run = 0;
        }
        flush(STDOUT);
    }
}
