#include "io.h"
#include "kernel.h"
#include "paging.h"

#define BUFFER_SIZE 0x800
#define KEY_SQUARE_SUPERSCRIPT 41  
#define KEY_STAR 43

u8 eventBuffer[BUFFER_SIZE];
u16 writeCursor = 0;
u16 readCursor = 0;

u8 keyboardState() {
    return inportb(0x60);
}

void provideKeyEvent(u8 event) {    
    if(event < 0x80 && event == KEY_SQUARE_SUPERSCRIPT) focus_next_process();
    else if (event == KEY_STAR) {
        terminal_color = make_color(COLOR_WHITE, COLOR_BLUE);
        for (int i = 0; i < 25; i++){
            kprintf("\n");
        }
        terminal_row = 0;
        terminal_column = 0;
        log_state(&global_state);
        print_malloc();
        asm("hlt");
    }
    else if(writeCursor != (readCursor-1) % BUFFER_SIZE) {
        eventBuffer[writeCursor] = event;
        writeCursor = (writeCursor + 1) % BUFFER_SIZE;
    }  
}

int nextKeyEvent() {
    if(readCursor == writeCursor)
        return -1;
    else {
        int event = eventBuffer[readCursor];
        readCursor = (readCursor + 1) % BUFFER_SIZE;
        return event;
    }
}
