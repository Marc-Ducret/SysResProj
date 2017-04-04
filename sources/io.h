#ifndef IO_H
#define IO_H
#include "int.h"
u8 inportb(u16 _port);
void outportb(u16 _port, u8 _data);
void wait_key(u8 k);

#endif /* IO_H */

