#include "isr.h"
#include "printing.h"
#include "io.h"
#include "keyboard.h"

void test() {
    putchar('.');
}

void schedule() {
    //TODO
}

void syscall(int id) {
    //TODO
}

void isr_handler(int id) {
    //TODO Gérer les interruptions importantes !
    kprintf("Caught interruption %d\n", id);
    // Peut etre pas besoin de s'en acquitter si c'est une interruption ?
    //outportb(0x20,0x20);
    //outportb(0xA0,0x20);
}

void irq_handler(int id) {
    
    if (id == 0) { // Timer
        schedule();
    }
    if(id == 1) { // Keyboard
        while ((inportb(0x64) & 0x01) == 0);
        provideKeyEvent(inportb(0x60));
    }
    
    // Informs the PIC we are done with this IRQ
    outportb(0x20,0x20);
    
    if (id > 7) //Only tells the slave PIC for IRQ 8 .. IRQ 15
        outportb(0xA0,0x20);
}

