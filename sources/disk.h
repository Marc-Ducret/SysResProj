#ifndef DISK_H
#define DISK_H
#include "int.h"
#include "printing.h"
#include <stddef.h>
#include "io.h"

#define PRIMARY_BUS 0x1F0
#define SECONDARY_BUS 0x170

typedef struct {
    u32 err : 1;
    u32 unused : 2;
    u32 drq : 1;
    u32 srv : 1;
    u32 df : 1;
    u32 ready : 1;
    u32 busy : 1;
} __attribute__ ((packed)) status_byte;

typedef union {
    u8 content;
    status_byte bits;
} status_byte_bis;

typedef struct {
    u16 data_port;
    u16 error_port;
    u16 sector_count;
    u16 lba_lo;
    u16 lba_mid;
    u16 lba_hi;
    u16 drive_select;
    u16 command_port;
    u16 control_register;
} bus;
bus *pbus;
void create_bus();
int disk_identify();
void init_disk();

#endif /* DISK_H */

