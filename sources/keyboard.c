#include "io.h"
#include "keycode.h"
#include "kernel.h"
#include "paging.h"

#define BUFFER_SIZE 0x800

u8 keyboardState() {
    return inportb(0x60);
}

void provideKeyEvent(u8 event) {
    event = event;
    //process p = get_global_state()->processes[get_global_state()->focus];
    //void *keyBuff = get_physical(get_page(USER_KEYBUFFER_VIRTUAL, 0, p.page_directory));
    //u16 *writeCursor = (u16 *) keyBuff;
    //u16 *readCursor = (u16 *) keyBuff + 1;
    //u8 *eventBuffer = (u8 *) keyBuff + 4;
    if(event < 0x80 && event == KEY_TAB) focus_next_process();
    else {
        //eventBuffer[*writeCursor] = event;
        //*writeCursor = (*writeCursor + 1) % BUFFER_SIZE;
    }  
}
