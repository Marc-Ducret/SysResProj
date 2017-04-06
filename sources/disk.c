#include "disk.h"

bus primary_bus;
bus secondary_bus;

bus *pbus = &primary_bus;
bus *sbus = &secondary_bus;

// TODO Clear control register ?

void init_bus(bus *bus, u16 base_port) {
    bus->data_port = base_port;
    bus->error_port = base_port + 1;
    bus->sector_count = base_port + 2;
    bus->lba_lo = base_port + 3;
    bus->lba_mid = base_port + 4;
    bus->lba_hi = base_port + 5;
    bus->drive_select = base_port + 6;
    bus->command_port = base_port + 7;
    bus->control_register = base_port + 0x206;
}

void create_bus() {
    init_bus(pbus, PRIMARY_BUS);
    init_bus(sbus, SECONDARY_BUS);
}

int disk_identify() {
    u8 status;
    
    // Master drive
    outportb(pbus->drive_select, 0xA0);
    
    outportb(pbus->sector_count, 0x0);
    outportb(pbus->lba_lo, 0x0);
    outportb(pbus->lba_mid, 0x0);
    outportb(pbus->lba_hi, 0x0);
    
    // Identify command
    outportb(pbus->command_port, 0xEC);
    
    status = inportb(pbus->command_port);
    
    if (!status) {
        // No drive..
        kprintf("The drive doesn't exist..\n");
        return 0;
    }
    else {
        kprintf("Found a drive !\n");
        while (inportb(pbus->command_port) & 0x80) {}
        
        //Checks the LBAmid and LBAhi
        if (inportb(pbus->lba_mid) || inportb(pbus->lba_hi)) {
            kprintf("Not ATA drive.\n");
            return 1;
        }  
        
        while (!(inportb(pbus->command_port) & 0x09)) {}
        
        //Checks error bit is clear
        if (inportb(pbus->command_port) & 0x1) {
            kprintf("Error bit was set.\n");
            return 3;
        }
        u16 table[256];
        for (int i = 0; i < 256; i++) {
            table[i] = inportw(pbus->data_port);
        }
        kprintf("Driver properties :\n");
        
        if ((table[88] >> 10) & 1) {
            kprintf(" - This drive supports LBA48 mode.\n");
        }
        else {
            kprintf(" - This drive does not support LBA48 mode.\n");
        }
        
        u32 lba28_sectors = table[60] | (table[61] << 16);
        kprintf(" - There are %d 28-bit LBA addressable sectors.\n", lba28_sectors);
        return 2;
    }
}

status_byte get_status(bus *bus) {
    status_byte_bis status;
    status.content = inportb(bus->command_port);
    return status.bits;
}

int is_busy(status_byte status) {
    return !(!status.busy || status.err || status.df);
}

int is_ready(status_byte status) {
    return !(status.err || status.df);
}

int poll(bus *bus) {
    // Just polls until drive is ready.
    while(is_busy(get_status(bus))) {}
    //do {
        //inportb(bus->command_port);
        //inportb(bus->command_port);
        //inportb(bus->command_port);
        //inportb(bus->command_port);
    //} while (is_busy(get_status(bus)));
    return is_ready(get_status(bus));
}

void software_reset(bus *bus) {
    // Sets the SRST bit of control register.
    outportb(bus->control_register, 0x4);
    //Clears the control register
    outportb(bus->control_register, 0x0);
}

void read_sectors(u32 sector, u8 sector_count, bus *bus, u16* buffer) {
    outportb(bus->drive_select, 0xE0 | ((sector >> 24) & 0x0F));
    //outportb(bus->error_port, 0x0);
    outportb(bus->sector_count, sector_count);
    outportb(bus->lba_lo, (u8) sector);
    outportb(bus->lba_mid, (u8) (sector >> 8));
    outportb(bus->lba_hi, (u8) (sector >> 16));
    outportb(bus->command_port, 0x20);
    
    for (int i = 0; i < sector_count; i++) {
        // Waiting for an IRQ, or polling. (Here polling)
        int ready = poll(bus);
        
        // Transfer the data from port to buffer.
        for (int j = 0; j < 256; j++) {
            buffer[j] = inportw(bus->data_port);
        }
        
        buffer += 256;
    }
}

void write_sectors(u32 sector, u8 sector_count, bus *bus, u16* buffer) {
    outportb(bus->drive_select, 0xE0 | ((sector >> 24) & 0x0F));
    //outportb(bus->error_port, 0x0);
    outportb(bus->sector_count, sector_count);
    outportb(bus->lba_lo, (u8) sector);
    outportb(bus->lba_mid, (u8) (sector >> 8));
    outportb(bus->lba_hi, (u8) (sector >> 16));
    outportb(bus->command_port, 0x30);
    
    for (int i = 0; i < sector_count; i++) {
        // Waiting for an IRQ, or polling. (Here polling)
        int ready = poll(bus);
        // Transfer the data from port to buffer.
        for (int j = 0; j < 256; j++) {
            outportw(bus->data_port, buffer[j]);
        }
        
        buffer += 256;
    }
    kprintf("\n");
    outportb(bus->command_port, 0xE7);
    poll(bus);
}

void init_disk() {
    create_bus();
    software_reset(pbus);
    int res = disk_identify();
    outportb(pbus->control_register, 0x0);
    u16 buffer[256];
    
    if (res == 2) {
        /*
        read_sectors(0x0, 1, pbus, buffer);
        for (int i = 0; i < 20; i++) {
            kprintf("%h ", buffer[i]);
        }
        kprintf("...\n");
        kprintf("Status after first read : %x\n", inportb(pbus->command_port));
        
        memset((void*)buffer, 0x13, 512);
        write_sectors(43, 1, pbus, buffer);
        kprintf("Status after writing : %x\n", inportb(pbus->control_register));
        
        read_sectors(42, 1, pbus, buffer);
        kprintf("New read sector :\n");
        for (int i = 0; i < 20; i++) {
            kprintf("%h ", buffer[i]);
        }
        kprintf("...\n");
        kprintf("Status after second read : %x\n", inportb(pbus->command_port));
        */
    }
}