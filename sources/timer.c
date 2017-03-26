#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "lib.h"
#include "int.h"
#include "printing.h"

void init_timer(u32 frequency) {
    u32 divisor = 1193180 / frequency;
    
    if (divisor == 0 || divisor >> 16 != 0) {
        kprintf("Failed to init the time : invalid divisor %d\n", divisor);
        return;
    }
    
    // Send the command byte.
    outportb(0x43, 0x36);
    
    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)( (divisor>>8) & 0xFF );
    
    // Send the frequency divisor.
    outportb(0x40, l);
    outportb(0x40, h);
    
    kprintf("Timer set to frequency %d Hz\n", frequency);
}