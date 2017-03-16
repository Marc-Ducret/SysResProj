#ifndef IO_H
#define IO_H

unsigned char inportb(unsigned short _port);
void outportb(unsigned short _port, unsigned char _data);
void wai_key(unsigned char k);

#endif /* IO_H */

