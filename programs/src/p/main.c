#include "lib.h"

u8 recv_buff[513];

int main() {
    clear_screen(COLOR_BLUE);
    u8 c = '%';
    if(send(1, &c, 1) < 0) {
        int err = get_error();
        set_char_at('0'+err/10, COLOR_GREEN, COLOR_BLUE, 0, 0);
        set_char_at('0'+err%10, COLOR_GREEN, COLOR_BLUE, 1, 0);
        for(;;);
    }
    for(u8 i = 0;; i++) {
        clear_screen(i % 0x10);
        
        int ct;
        if((ct = receive(0, recv_buff, 512)) > 0) {
            recv_buff[ct] = 0;
            printf("%s", recv_buff);
            flush(STDOUT);
        }
    }
}
