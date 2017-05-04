#include "lib.h"

int main() {
    for(u8 i = 0; i != 0xFF; i++) *((u16*)0x88000000 + i) = 0x0200 + i;
    for(u32 i = 0;; i++) {
        int event = kget_key_event();
        if(event >= 0 && event < 0x80) {
            set_char_at('0' + (event % 10), COLOR_RED, COLOR_WHITE, 5, 5);
        }
    }
}
