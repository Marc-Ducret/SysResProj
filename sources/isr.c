#include "isr.h"
#include "printing.h"

void isr_handler(struct registers r) {
    kprintf("Error number %d \n", r.int_no);
    kprintf("Error code %d", r.err_code);
}

