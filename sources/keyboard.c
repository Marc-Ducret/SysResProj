#include "io.h"
#include "keycode.h"
#include "kernel.h"

#define BUFFER_SIZE 1024

int eventBuffer[BUFFER_SIZE];
int eventCursor = 0;

int keyboardState() {
    return inportb(0x60);
}

void provideKeyEvent(int event) {
    if(event < 128 && event == KEY_TAB) focus_next_process();
    else eventBuffer[eventCursor++] = event;
}

int nextKeyEvent() {
    if(eventCursor > 0)
        return eventBuffer[--eventCursor];
    return -1;
}
