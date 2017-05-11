#include "lib.h"

int main() {
    printf("Hello!\n");
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
    u32 m = 0x200000;
    void ** pointers = (void **) malloc(n * sizeof(void **));
    for(int i = 0; i < n; i ++) pointers[i] = NULL;
    for(u32 ct = 0; ct < 0x10000; ct++) {
        int i = rand() % n;
        if(pointers[i]) free(pointers[i]);
        pointers[i] = malloc(rand() % m);
        flush(STDOUT);
        if(!pointers[i]) {
            printf("fail\n");
            exit(-1);
        }
    }
    
    printf("success\n");
}
