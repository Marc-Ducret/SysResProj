#include "io.h"

#define BUFFER_SIZE 1024

int eventBuffer[BUFFER_SIZE];
int eventCursor = 0;

int keyboardState() {
    return inportb(0x60);
}

void provideKeyEvent(int event) {
    eventBuffer[eventCursor++] = event;
}

int nextKeyEvent() {
    if(eventCursor > 0)
        return eventBuffer[--eventCursor];
    return -1;
}
