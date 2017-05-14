#ifndef IO_H
#define IO_H
#include "int.h"
u8 inportb(u16 _port);
void outportb(u16 _port, u8 _data);
u16 inportw(u16 _port);
void outportw(u16 _port, u16 _data);

#endif /* IO_H */

